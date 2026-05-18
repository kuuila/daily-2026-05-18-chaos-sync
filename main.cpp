// chaos-sync: 混沌同步 - 两只蝴蝶的同步之舞
// 两个Lorenz系统通过微弱耦合达到同步 - 混沌中的秩序
// 蝴蝶效应: 初始条件相差0.001，经过足够时间，两只蝴蝶完美同步飞舞
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <random>

constexpr float SIGMA = 10.0f;
constexpr float BETA = 8.0f / 3.0f;
constexpr float RHO = 28.0f;
constexpr float DT = 0.004f;
constexpr float COUPLE = 0.02f; // 耦合强度，极弱

struct Lorenz { float x, y, z; };

void rk4(Lorenz& s, float dt, float kx, float ky) {
    float dx1 = SIGMA * (s.y - s.x), dy1 = s.x * (RHO - s.z) - s.y, dz1 = s.x * s.y - BETA * s.z;
    float x2 = s.x + dx1 * dt * 0.5, y2 = s.y + dy1 * dt * 0.5, z2 = s.z + dz1 * dt * 0.5;
    float dx2 = SIGMA * (y2 - x2), dy2 = x2 * (RHO - z2) - y2, dz2 = x2 * y2 - BETA * z2;
    x2 = s.x + dx2 * dt * 0.5; y2 = s.y + dy2 * dt * 0.5; z2 = s.z + dz2 * dt * 0.5;
    float dx3 = SIGMA * (y2 - x2), dy3 = x2 * (RHO - z2) - y2, dz3 = x2 * y2 - BETA * z2;
    x2 = s.x + dx3 * dt; y2 = s.y + dy3 * dt; z2 = s.z + dz3 * dt;
    float dx4 = SIGMA * (y2 - x2), dy4 = x2 * (RHO - z2) - y2, dz4 = x2 * y2 - BETA * s.z;
    s.x += (dx1 + 2 * dx2 + 2 * dx3 + dx4) * dt / 6.0f;
    s.y += (dy1 + 2 * dy2 + 2 * dy3 + dy4) * dt / 6.0f;
    s.z += (dz1 + 2 * dz2 + 2 * dz3 + dz4) * dt / 6.0f;
    // 耦合项
    s.x += kx * dt; s.y += ky * dt;
}

int main() {
    sf::RenderWindow win(sf::VideoMode(900, 700), "Chaos Sync - 混沌同步", sf::Style::Close);
    win.setFramerateLimit(60);

    std::vector<sf::VertexArray> traces(2);
    traces[0].setPrimitiveType(sf::LineStrip);
    traces[1].setPrimitiveType(sf::LineStrip);

    std::vector<std::vector<sf::Vector2f>> bufs(2);
    bufs[0].reserve(2000);
    bufs[1].reserve(2000);
    Lorenz a={1.0f,1.0f,1.0f}, b={1.001f,1.0f,1.0f};
    std::vector<Lorenz> lz;
    lz.push_back(a); lz.push_back(b); // 仅x差0.001
    std::vector<sf::CircleShape> dots(2);
    for (int i = 0; i < 2; i++) { dots[i].setRadius(4); dots[i].setOrigin(4, 4); dots[i].setFillColor(i ? sf::Color(100, 200, 255) : sf::Color(255, 100, 150)); }

    sf::Font font; bool fontOK = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
    sf::Text info, syncLabel; info.setCharacterSize(13); info.setFillColor(sf::Color(180, 180, 180)); info.setPosition(15, 15);
    syncLabel.setCharacterSize(15); syncLabel.setFillColor(sf::Color(255, 255, 100)); syncLabel.setPosition(15, 580);
    if (fontOK) { info.setFont(font); syncLabel.setFont(font); }

    bool startPause = false; int steps = 0, lastSyncFrame = 0;
    sf::VertexArray grid(sf::Lines); sf::Color gridCol(40, 40, 60);
    for (int x = 100; x <= 800; x += 100) { grid.append(sf::Vertex(sf::Vector2f(x, 50), gridCol)); grid.append(sf::Vertex(sf::Vector2f(x, 550), gridCol)); }
    for (int y = 50; y <= 550; y += 100) { grid.append(sf::Vertex(sf::Vector2f(100, y), gridCol)); grid.append(sf::Vertex(sf::Vector2f(800, y), gridCol)); }

    while (win.isOpen()) {
        sf::Event e; while (win.pollEvent(e)) if (e.type == sf::Event::Closed) win.close();
        if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Space) startPause = !startPause;

        if (!startPause) {
            float cx0 = lz[0].x, cy0 = lz[0].y;
            rk4(lz[0], DT, 0, 0);
            float cx1 = lz[1].x, cy1 = lz[1].y;
            rk4(lz[1], DT, COUPLE * (cx0 - cx1), COUPLE * (cy0 - cy1));

            for (int i = 0; i < 2; i++) {
                float sx = lz[i].x / 2.0f, sy = lz[i].z / 12.0f - 15.0f;
                sf::Vector2f pt(lz[i].y * 9.0f + (i ? 450.f : 450.f), lz[i].x * 5.0f + 300.f);
                if (pt.x > 100 && pt.x < 800 && pt.y > 50 && pt.y < 550) {
                    bufs[i].push_back(pt);
                    if (bufs[i].size() > 1200) bufs[i].erase(bufs[i].begin());
                    traces[i].resize(bufs[i].size());
                    for (size_t j = 0; j < bufs[i].size(); j++) {
                        float a = 80 + (float)j / bufs[i].size() * 175;
                        traces[i][j] = sf::Vertex(bufs[i][j], sf::Color(a > 255 ? 255 : (int)a, 50, i ? 200 : 150));
                    }
                    dots[i].setPosition(lz[i].y * 9.0f + 450.f - 4.f, lz[i].x * 5.0f + 300.f - 4.f);
                }
            }

            float dist = std::sqrt((lz[0].x - lz[1].x) * (lz[0].x - lz[1].x) + (lz[0].y - lz[1].y) * (lz[0].y - lz[1].y) + (lz[0].z - lz[1].z) * (lz[0].z - lz[1].z));
            bool synced = dist < 0.5f;
            if (synced) lastSyncFrame++;
            else lastSyncFrame = 0;

            info.setString("混沌同步演示 | 空格键暂停/继续\n\n蝴蝶1 (粉) 初始: (1, 1, 1)\n蝴蝶2 (蓝) 初始: (1.001, 1, 1)\n差异: 0.001\n耦合强度: " + std::to_string(COUPLE) +
                "\n\n当前两蝴蝶距离: " + std::to_string(dist) +
                "\n已同步帧: " + std::to_string(lastSyncFrame) +
                "\n总步数: " + std::to_string(steps));
            syncLabel.setString(synced ? "【已同步】两只蝴蝶完美共舞！" : "【未同步】寻找同步中...");
            if (synced) syncLabel.setFillColor(sf::Color(100, 255, 150)); else syncLabel.setFillColor(sf::Color(255, 200, 100));
            steps++;
        }

        win.clear(sf::Color(8, 8, 18));
        win.draw(grid);
        for (auto& t : traces) win.draw(t);
        for (auto& d : dots) win.draw(d);
        win.draw(info); win.draw(syncLabel);
        win.display();
    }
    return 0;
}
