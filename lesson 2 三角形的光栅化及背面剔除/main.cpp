#include <vector> 
#include <iostream> 
#include "geometry.h"
#include "tgaimage.h" 
#include "model.h"

const int width = 800;
const int height = 800;


Vec3f barycentric(Vec2i* pts, Vec2i P) {
    Vec3f u = Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]) ^ Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]);
    /* `pts` and `P` has integer value as coordinates
       so `abs(u[2])` < 1 means `u[2]` is 0, that means
       triangle is degenerate, in this case return something with negative coordinates */
    if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(Vec2i pts[], TGAImage& image, TGAColor color) {
    Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    Vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            image.set(P.x, P.y, color);
        }
    }
}

Vec3f light_dir(0, 0, -1);
int main(int argc, char** argv) {
    TGAImage image(width, height, TGAImage::RGB);
    Model* model = new Model("E:\\tiny_renderer_dir\\models\\obj\\african_head.obj");

    for (int i = 0; i < model->nfaces(); i++) {
        // face变量存储了这个面的三个顶点序号
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j = 0; j < 3; j++) {
            world_coords[j] = model->vert(face[j]);
            screen_coords[j] = Vec2i((world_coords[j].x + 1.) * width / 2., (world_coords[j].y + 1.) * height / 2.);
        }
        Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        n.normalize();
        light_dir.normalize();
        float intensity = n * light_dir;
        if (intensity <= 0)
            continue;
        // rgb被设置成了随机值
        triangle(screen_coords, image, TGAColor((rand() % 255) * intensity, (rand() % 255) * intensity, (rand() % 255)* intensity, 255));
    }

    image.flip_vertically(); // to place the origin in the bottom left corner of the image 
    image.write_tga_file("framebuffer.tga");
    return 0;
}