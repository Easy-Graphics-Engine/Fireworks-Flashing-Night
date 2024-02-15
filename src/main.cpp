#include <graphics.h>

#include <time.h>
#include <math.h>
#include <Windows.h>

#define GROUND 580  //地面位置y坐标

// 速度
struct Speed
{
    double x, y;
};

// 位置
struct Pos
{
    double x, y;
};

// 粒子
struct Particle
{
    Pos pos;
    Speed speed;
};

// 烟花
class Fireworks
{
private:
    static const int NUM_PARTICLE = 200;
    static const double particleSpeed;
    Particle p[NUM_PARTICLE];

    color_t color;

    int delayTime;      //延迟时间
    int riseTime;       //上升时间
    int bloomTime;      //爆炸时间

    Pos risePos;        //上升阶段位置
    Speed riseSpeed;    //上升速度

public:
    //初始化
    Fireworks();

    void init();

    //更新位置等相关属性
    void update();

    //根据属性值绘画
    void draw(PIMAGE pimg = NULL);
};

const double Fireworks::particleSpeed = 3.0;

Fireworks::Fireworks()
{
    init();
}

void Fireworks::init()
{
    delayTime = rand() % 300 + 20;
    riseTime = rand() % 80 + 160;
    bloomTime = 160;

    risePos.x = rand() % 450 + 300.0f;
    risePos.y = GROUND;

    riseSpeed.y = randomf() * 1.0 - 3.0; //速度 y: [-3.0, -2.0), 根据坐标系需要是负的
    riseSpeed.x = randomf() * 0.4 - 0.2; //速度 x: [-0.2, 0.2), 可稍微倾斜

    //随机颜色
    color = HSVtoRGB((float)randomf() * 360.0, 1.0f, 1.0f);

    //给每一个粒子设置初始速度
    for (int i = 0; i < NUM_PARTICLE - 1; i += 2)
    {
        //为了球状散开，设初始速度大小相等
        //初始随机速度水平角度和垂直角度，因为看到是平面的，所以求x, y分速度
        double levelAngle = randomf() * 360;
        double verticalAngle = randomf() * 360;

        //速度投影到xOy平面
        double xySpeed = particleSpeed * cos(verticalAngle);

        //求x, y分速度
        p[i].speed.x = xySpeed * cos(levelAngle);
        p[i].speed.y = xySpeed * sin(levelAngle);

        //动量守恒，每对速度反向
        if (i + 1 < NUM_PARTICLE) {
            p[i + 1].speed.x = -p[i].speed.x;
            p[i + 1].speed.y = -p[i].speed.y;
        }
    }
}

void Fireworks::draw(PIMAGE pimg)
{
    //未开始
    if (delayTime > 0)
        return;
    //烟花上升阶段
    else if (riseTime > 0) {
        setfillcolor(color, pimg);
        //画四个点，这样大一些
        bar(risePos.x, risePos.y, risePos.x + 2, risePos.y + 2, pimg);
    }
    //烟花绽放阶段
    else {
        setfillcolor(color, pimg);
        for (int i = 0; i < NUM_PARTICLE; i++) {
            bar(p[i].pos.x, p[i].pos.y, p[i].pos.x + 2, p[i].pos.y + 2, pimg);
        }
    }
}

//更新位置等相关属性
void Fireworks::update()
{
    if (delayTime-- > 0)
        return;
    //处于上升阶段，只更新烟花位置
    else if (riseTime > 0) {
        risePos.x += riseSpeed.x;
        risePos.y += riseSpeed.y;

        //重力作用
        riseSpeed.y += 0.005;

        //上升完毕，到达爆炸阶段
        if (--riseTime <= 0) {
            //设粒子初始位置为烟花当前位置
            for (int i = 0; i < NUM_PARTICLE; i++) {
                p[i].pos.x = risePos.x;
                p[i].pos.y = risePos.y;
            }
        }
    }
    //烟花绽放阶段
    else if (bloomTime-- > 0) {
        //粒子散开，更新粒子位置
        for (int i = 0; i < NUM_PARTICLE; i++) {
            p[i].pos.x += p[i].speed.x;
            p[i].pos.y += p[i].speed.y;

            //重力作用
            p[i].speed.y += 0.005;

            //速度减慢
            p[i].speed.x *= 0.982;
            p[i].speed.y *= 0.982;
        }
    }
    else {
        //烟花重新开始
        init();
    }
}


#define NUM_FIREWORKS 12    //烟花数量，12左右比较好，多了太密集

int main()
{

    initgraph(800, 800, INIT_RENDERMANUAL);

    randomize();

    //烟花
    Fireworks* fireworks = new Fireworks[NUM_FIREWORKS];

    //背景图（800 x 800)
    PIMAGE bgPimg = newimage();
    getimage(bgPimg, "resources/background.jpg");
    //先绘制一下，不然前面有空白期
    putimage(0, 0, bgPimg);
    delay_ms(0);

    //背景音乐
    MUSIC bgMusic;
    bgMusic.OpenFile("resources/background_music.mp3");
    bgMusic.SetVolume(1.0f);
    if (bgMusic.IsOpen()) {
        bgMusic.Play(0);
    }


    //图像缓存, 因为要加背景图，直接加模糊滤镜会把背景图模糊掉
    //所以另设一个图像缓存来绘制烟花并加模糊滤镜，再绘制到窗口
    PIMAGE cachePimg = newimage(800, 800);

    //计时用，主要用来定时检查音乐播放
    int timeCount = 0;

    for (; is_run(); delay_fps(60))
    {
        //隔1秒检查一下，如果播放完了，重新播放
        if ((++timeCount % 60 == 0) && (bgMusic.GetPlayStatus() == MUSIC_MODE_STOP)) {
            bgMusic.Play(0);
        }
        //更新位置
        for (int i = 0; i < NUM_FIREWORKS; i++) {
            fireworks[i].update();
        }

        //清屏
        cleardevice();
        //绘制背景
        putimage(0, 0, bgPimg);

        //绘制烟花到图像缓存中
        for (int i = 0; i < NUM_FIREWORKS; i++) {
            fireworks[i].draw(cachePimg);
        }

        //模糊滤镜，拖尾效果
        //第二个参数，模糊度，越大越模糊，粒子也就越粗
        //第三个参数，亮度，越大拖尾越长
        //可以试试一下其它参数搭配，例如以下几组：
        //0x03, 0xff
        //0x0b, 0xe0
        //0xff, 0xff
        imagefilter_blurring(cachePimg, 0x0a, 0xff);

        //缓存绘制到窗口，模式为（最终颜色 = 窗口像素颜色 Or 图像像素颜色), 这样颜色会叠加起来
        putimage(0, 0, cachePimg, SRCPAINT);

    }

    //释放动态分配的内存
    delete[] fireworks;

    //删除图像，释放资源
    delimage(bgPimg);
    delimage(cachePimg);

    //释放音频资源
    bgMusic.Close();

    closegraph();

    return 0;
}
