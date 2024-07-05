# Render System

## Material System

## Lighting

| Photometric term | Notation	| Unit |
| --- | :---: | --- |
| Luminous power | $\Phi$ | Lumen ($lm$) |
| Luminous intensity | $I$ | Candela ($cd$) or $\frac{lm}{sr}$ |
| Illuminance | $E$ | Lux($lx$) or $\frac{lm}{m^2}$ |
| Luminance | $L$ | Nit ($nt$) or $\frac{cd}{m^2}$ |

### Point Light


## Imaging Pipeline
### Physically based camera

| 参数 | 定义 | 表示 |
| --- | --- | --- |
| focal length | 焦距, 毫米为单位 | $f$ |
| Aperture | 用f-stops F(焦比)表示, $\frac{f}{d}$ , $f$是焦距, $d$光圈直径, 描述了光圈的打开/闭合程度. | $N$, 全级级数光圈有: f/1、f/1.4、f/2、f/2.8、f/4、f/5.6、f/8、f/11、f/16、f/22. 这些光圈值是公比为 $\frac{1}{\sqrt{2}}$  |
| Shutter speed | 快门速度 | $t$ |
| Sensitivity/ISO | 感光度, 高ISO虽然速度快但图像颗粒粗，经不起精细放大出图 | $S$ |

参考[Exposure_value](https://en.wikipedia.org/wiki/Exposure_value), Exposure可以通过Aperture, Shutter speed, Sensitivity的值计算得到, 值越小灵敏度越高:


$$
\begin{aligned}
EVs &= \log_2\frac{N^2}{t}\\
EV100 &= \log_2(\frac{N^2}{t} \frac{100}{S})\\
EV100 &= EVs - \log_2 \frac{S}{100}
\end{aligned}
$$

参考[Film speed](https://en.wikipedia.org/wiki/Film_speed), ev100, 照片上的曝光值$H$计算如下($q=0.65$):

$$
H = \frac{q \cdot t}{N^2} \cdot L = q\frac{1}{2^{EV100}} \cdot L
$$

H的单位是($lx/s$)
在一定的光照下, 自动曝光(标准曝光)可以根据如下公式计算:

$$
\frac{N^2}{t} = \frac{LS}{K} = \frac{ES}{C}
$$

$L$是反射的luminance, $K=12.5 \mathrm{cd\;s/(m\;ISO)}^2$ 是light meater的标定值.
$E$是illuminance, $C=250 \mathrm{lm\;s/(m^2\;ISO)}$ 是incident-light meter标定值.

filament参考: View.cpp->mPerViewUniforms.prepareDirectionalLight
[ue参考](https://docs.unrealengine.com/5.3/en-US/auto-exposure-in-unreal-engine/)

在系统中, 相机只有一个ev100的参数.

### Coordinate system
全部使用右手坐标系

相机坐标系:
![](imgs/camera_coordinate.png)
