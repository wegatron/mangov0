# cpp-debugger-cli 使用经验

基于 `dbgeng.dll` 的 Python 调试 API，适合对 Windows 原生 C++ 程序做自动化/脚本化调试。

**环境准备：**
```python
import sys, os
sys.path.insert(0, r"D:\workspace\cpp-debugger-cli\src")
from debugger.agent_api import debug_crash, debug_with_breakpoints, inspect_function
```

---

## API 速查

| 场景 | API | 说明 |
|------|-----|------|
| 定位崩溃 | `debug_crash(exe)` | 运行到崩溃，返回异常类型、调用栈、寄存器 |
| 断点收集上下文 | `debug_with_breakpoints(exe, [...])` | 在指定位置停下，收集局部变量和调用栈 |
| 逐行追踪函数 | `inspect_function(exe, func, steps=N)` | 返回每步的源码行和变量值 |
| Tracepoint（不停止） | `debug_with_breakpoints` + `auto_continue=True, command="..."` | 静默记录，程序不中断 |

---

## 启动程序前的环境配置

`debug_crash` / `debug_with_breakpoints` 内部调用 `dbgeng.dll` 的 `CreateProcess`，继承当前 Python 进程的环境变量和工作目录。**启动调试前必须正确设置：**

### 1. 工作目录

程序依赖相对路径加载资源（shader、配置文件等），必须从程序预期的工作目录启动。查阅 `.vscode/launch.json` 的 `cwd` 字段确认：

```python
os.chdir(r"D:\workspace\project\run")  # 在调用任何 debug_* API 之前
```

若未设置，`debug_crash` 会报 `NO_DEBUGGEE`（进程启动失败），而非调试错误。

### 2. DLL 路径

第三方 DLL 若不在系统 PATH 中，进程会在启动阶段即退出（`NO_DEBUGGEE`）。在调用前追加到 PATH：

```python
os.environ["PATH"] = r"D:\workspace\project\thirdparty\install\bin;" + os.environ["PATH"]
```

### `NO_DEBUGGEE` 排查思路

```
RuntimeError: Expected BREAK after launch, got NO_DEBUGGEE
```

出现此错误说明进程根本没有启动成功，与程序逻辑无关。检查顺序：
1. exe 路径是否正确
2. 缺少 DLL → 补充 PATH
3. 工作目录不对导致资源找不到 → 设置 `os.chdir`

---

## `debug_crash` — 崩溃分析

```python
result = debug_crash(exe)

print(result["status"])           # "crashed" / "exited"
print(result["summary"])          # 一行摘要，如 "CPP_EXCEPTION — in CxxThrowException"
print(result["exception_type"])   # "CPP_EXCEPTION" / "ACCESS_VIOLATION" / ...
print(result["crash_location"])   # {"module", "function", "source_file", "source_line"}

# 过滤出项目自身的调用栈帧
for frame in result["stack"]:
    if "my_module" in frame.get("symbol", ""):
        print(frame["function"], "@", frame.get("source_file"), ":", frame.get("source_line"))
```

**适用场景**：程序会崩溃，想快速知道在哪、为什么。

**注意**：`debug_crash` 运行时会阻塞直到崩溃或超时（默认 30s）。对于正常运行的 GUI 程序，超时后会报 `"status": "crashed"` 但 `exception_type` 为 `None`，此时应改用 `debug_with_breakpoints`。

---

## `debug_with_breakpoints` — 断点调试

```python
result = debug_with_breakpoints(
    exe,
    [
        "my_function",                    # 函数名断点
        "foo.cpp:42",                     # 文件行断点
        {"location": "bar", "condition": "@rcx > 0n10"},          # 条件断点
        {"location": "log_func", "auto_continue": True,
         "command": r'.printf "called\n"'},                        # Tracepoint
        {"location": "init", "hit_count": 3},                     # 第3次命中才停
    ],
    max_stops=5
)

for stop in result["stops"]:
    print(stop["location"])
    print(stop["file"], ":", stop["line"])
    for var in stop["locals"]:
        print(" ", var["name"], "=", var["value"])
```

**适用场景**：
- 程序运行正常但行为不符合预期，想在特定位置观察状态
- 崩溃已定位到某函数，想捕获崩溃前的变量值
- GUI 程序初始化阶段的断点（避免 `debug_crash` 超时问题）

**`max_stops` 参数**：断点累计命中次数上限，到达后停止执行并返回。返回的 `status` 为 `"max_stops"` 表示正常结束，`"crashed"` 表示期间发生了崩溃。

---

## `inspect_function` — 逐步追踪

```python
result = inspect_function(exe, "mango::MainPass::init", steps=20)

for step in result["steps"]:
    print(f"line {step['line']}: {step['source']}")
    for var in step["locals"]:
        print(f"  {var['name']} = {var['value']}")
```

**适用场景**：想完整了解某个函数内部每一行的执行过程和变量变化。

---

## 底层 API（`DebugSession`）

`agent_api` 满足不了需求时，可直接使用 `DebugSession`：

```python
from debugger.engine import DebugSession

dbg = DebugSession()
dbg.launch(exe)
dbg.set_breakpoint("foo.cpp:100")
dbg.go()                              # 运行到断点

print(dbg.get_status_name())          # "BREAK"
print(dbg.get_locals())
print(dbg.evaluate("myVar + 1"))      # 表达式求值（MASM 语法）
print(dbg.get_stack())
print(dbg.get_registers())

dbg.step_over()
dbg.step_into()

dbg.close()
```

**`evaluate` 的限制**：使用 CDB 的 MASM 表达式求值器，不支持 C++ 成员函数调用（如 `obj.getLog()`）。如需求值复杂 C++ 表达式，改用 `execute("?? expr")` 执行原始 CDB 命令。

---

## GUI 程序调试注意事项

1. **不要用 `debug_crash` 等待 GUI 运行**：GUI 程序启动后会阻塞在消息循环，30s 超时后调试器强制终止，结果无意义。
2. **在初始化路径上设断点**：GUI 程序的 bug 往往在启动阶段，用 `debug_with_breakpoints` 在 `init` / `load` 等函数上设断点，命中后程序暂停，可以安全读取状态再继续。
3. **验证程序能正常通过某个检查点**：

```python
result = debug_with_breakpoints(exe, ["critical_init_function"], max_stops=1)
if result["status"] == "max_stops":
    print("✅ 初始化成功通过")
elif result["status"] == "crashed":
    print("❌ 初始化期间崩溃")
    # 查看 result["stack"] 定位问题
```
