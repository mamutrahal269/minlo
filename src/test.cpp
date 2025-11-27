#include <minlib.hpp>
#include <vbe.hpp>

void put_pixel(video_mode& vmode, u32 x, u32 y, u8 r, u8 g, u8 b) {
    u8* fb = reinterpret_cast<u8*>(vmode.framebuffer_addr);
    u32 pitch = vmode.framebuffer_pitch;
    u32* pixel = reinterpret_cast<u32*>(fb + y * pitch + x * 4);
    *pixel = (r << vmode.framebuffer_red_field_position)
           | (g << vmode.framebuffer_green_field_position)
           | (b << vmode.framebuffer_blue_field_position);
}
void __attribute__((section(".main"))) __attribute__((noreturn)) __attribute__((used)) main() {
    auto vmode = VBEmode_setup(mode_type::graphics, ~0, ~0, ~0);

    float angle = 0.0f;

    for (;;) {
        // очистка экрана
        for (u32 y = 0; y < vmode.framebuffer_height; y++)
            for (u32 x = 0; x < vmode.framebuffer_width; x++)
                put_pixel(vmode, x, y, 0, 0, 0);

        angle += 0.04f;

        float ca = __builtin_cosf(angle);
        float sa = __builtin_sinf(angle);

        // сетка: XZ-плоскость
        for (int i = -20; i <= 20; i++) {
            for (int j = -20; j <= 20; j++) {

                // позиция точки в 3D
                float x = i;
                float z = j;
                float y = 0;

                // вращение вокруг Z (наклон)
                float rx = x * ca - y * sa;
                float ry = x * sa + y * ca;
                float rz = z;

                // вращение вокруг Y (поворот плоскости)
                float rx2 = rx * ca + rz * sa;
                float rz2 = -rx * sa + rz * ca;

                // перспектива
                float dist = 6.0f;
                float f = 300.0f / (rz2 + dist);

                int sx = int(rx2 * f + vmode.framebuffer_width  / 2);
                int sy = int(ry *  f + vmode.framebuffer_height / 2);

                // рисуем квадраты сетки
                if (sx >= 0 && sx < (int)vmode.framebuffer_width &&
                    sy >= 0 && sy < (int)vmode.framebuffer_height)
                {
                    // чередование цветов (шахматы)
                    bool cell = ((i + j) & 1);
                    u8 c = cell ? 255 : 80;
                    put_pixel(vmode, sx, sy, c, c, c);
                }
            }
        }
    }
}




