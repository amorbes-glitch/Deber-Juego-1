#include <array>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <algorithm>
#include <cmath>
#endif

struct Option {
    std::string emoji;
    std::string text;
};

struct Activity {
    int level;
    std::string section;
    std::string prompt;
    std::string story;
    std::string characters;
    std::vector<Option> options;
    std::string flower;
};

const std::array<Activity, 9> activities{{
    {1, "Caritas que hablan",
     "Mira la cara de Lila. ¿Qué emoción notas?",
     "Lila encontró una flor brillante y sus ojos se iluminan. Las caras nos dan pistas.",
     "Lila  😄",
     {{"😊", "Alegría"}, {"😢", "Tristeza"}, {"😠", "Enojo"}, {"😨", "Miedo"}},
     "🌼"},
    {1, "Caritas que hablan",
     "¿Cómo podría sentirse el conejito?",
     "Su torre de bloques se cayó. Su boquita está hacia abajo y sus orejas se inclinan.",
     "Conejito  🐰  😢",
     {{"😊", "Alegría"}, {"😢", "Tristeza"}, {"😠", "Enojo"}, {"😨", "Miedo"}},
     "🌷"},
    {1, "Caritas que hablan",
     "¡Dos amigos tienen caras diferentes! ¿Qué emociones reconoces?",
     "El osito frunce el ceño y la ranita abre mucho los ojos. ¿Qué notas en sus caras?",
     "Osito  🐻 😠     Ranita  🐸 😨",
     {{"😊", "Alegría"}, {"😢", "Tristeza"}, {"😠", "Enojo"}, {"😨", "Miedo"}},
     "🌻"},
    {2, "Historias del jardín",
     "A Tomás se le perdió su juguete favorito. ¿Cómo podría sentirse?",
     "Tomás busca por todas partes y no encuentra su autito. Cada persona puede sentir algo distinto.",
     "Tomás  🦊  🧸",
     {{"😢", "Tristeza o desilusión"}, {"😠", "Enojo o frustración"},
      {"😟", "Preocupación"}, {"🙂", "Tal vez otra emoción"}},
     "🌺"},
    {2, "Historias del jardín",
     "Mara recibió una sorpresa agradable. ¿Qué podría sentir?",
     "Sus amigos prepararon una merienda especial para ella. ¡Mara no lo esperaba!",
     "Mara  🐿️  🎁",
     {{"😄", "Alegría"}, {"🤩", "Sorpresa y emoción"},
      {"🥰", "Cariño"}, {"🙂", "Tal vez otra emoción"}},
     "🌸"},
    {2, "Historias del jardín",
     "Nico quiere jugar, pero sus amigos están ocupados. ¿Cómo podría sentirse?",
     "Nico mira alrededor y busca con quién compartir. Quizá le gustaría tener compañía.",
     "Nico  🦔  🦋",
     {{"😔", "Soledad o tristeza"}, {"😕", "Inquietud"},
      {"🙂", "Puede estar bien un ratito a solas"},
      {"💭", "Otra emoción también es posible"}},
     "🌼"},
    {3, "Amigos que se acompañan",
     "Ves a una amiga triste. ¿Cómo podrías acompañarla?",
     "A Nina se le arrugó el dibujo con el que estaba muy ilusionada. Piensa qué le podría ayudar.",
     "Nina  🐼  🖍️",
     {{"👂", "Escucharla y preguntarle qué necesita"},
      {"🫶", "Ofrecer compañía o un abrazo si ella quiere"},
      {"🎨", "Preguntarle si quiere hacer otro dibujo juntas"},
      {"🧑‍🏫", "Buscar a un adulto de confianza"}},
     "🌷"},
    {3, "Amigos que se acompañan",
     "Sientes enojo porque algo no salió como querías. ¿Qué podrías probar?",
     "El enojo es una emoción válida. Una pausa puede ayudarnos a cuidarnos y pensar.",
     "Draco  🐲  🍃",
     {{"🌬️", "Respirar despacito en el Rincón de la calma"},
      {"🗣️", "Contar con palabras qué me molestó"},
      {"⏸️", "Pedir un momento tranquilo antes de hablar"},
      {"🧑‍🤝‍🧑", "Pedir ayuda a una persona de confianza"}},
     "🌻"},
    {3, "Amigos que se acompañan",
     "Dos amigos quieren el mismo columpio. ¿Cómo pueden resolverlo?",
     "A los dos les gustaría jugar primero. Pueden expresarse, escucharse y pedir ayuda.",
     "Osito  🐻     Conejito  🐰     🛝",
     {{"💬", "Hablar por turnos y buscar un acuerdo"},
      {"⏳", "Proponer usarlo un ratito cada uno"},
      {"👂", "Escuchar qué necesita cada amigo"},
      {"🧑‍🏫", "Pedir ayuda a un adulto de confianza"}},
     "🌸"}
}};

#ifdef _WIN32

namespace {
constexpr int WINDOW_WIDTH = 1100;
constexpr int WINDOW_HEIGHT = 850;

enum class Screen { Welcome, Playing, Finished };

Screen screen = Screen::Welcome;
std::size_t currentActivity = 0;
bool selectedOptions[4]{};
bool answered = false;
bool calmOpen = false;
int breathingPhase = 0;
int animationTick = 0;
int starCount = 0;

COLORREF color(int r, int g, int b) { return RGB(r, g, b); }

std::wstring wide(const std::string& value) {
    if (value.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    if (length <= 1) return {};
    std::wstring result(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, result.data(), length);
    result.pop_back();
    return result;
}

void fill(HDC dc, int left, int top, int right, int bottom, COLORREF shade) {
    HBRUSH brush = CreateSolidBrush(shade);
    RECT rect{left, top, right, bottom};
    FillRect(dc, &rect, brush);
    DeleteObject(brush);
}

void roundedBox(HDC dc, int left, int top, int right, int bottom,
                COLORREF background, COLORREF edge, int radius = 18) {
    HBRUSH brush = CreateSolidBrush(background);
    HPEN pen = CreatePen(PS_SOLID, 2, edge);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    RoundRect(dc, left, top, right, bottom, radius, radius);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void ellipse(HDC dc, int left, int top, int right, int bottom, COLORREF shade,
             COLORREF edge = RGB(255, 255, 255), int edgeWidth = 1) {
    HBRUSH brush = CreateSolidBrush(shade);
    HPEN pen = CreatePen(PS_SOLID, edgeWidth, edge);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    Ellipse(dc, left, top, right, bottom);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void line(HDC dc, int x1, int y1, int x2, int y2, COLORREF shade, int width = 3) {
    HPEN pen = CreatePen(PS_SOLID, width, shade);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    MoveToEx(dc, x1, y1, nullptr);
    LineTo(dc, x2, y2);
    SelectObject(dc, oldPen);
    DeleteObject(pen);
}

void text(HDC dc, const std::wstring& value, int left, int top, int right, int bottom,
          int size, COLORREF shade, UINT alignment = DT_CENTER | DT_VCENTER | DT_WORDBREAK,
          bool bold = false) {
    HFONT font = CreateFontW(-size, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE,
                             FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, shade);
    RECT rect{left, top, right, bottom};
    DrawTextW(dc, value.c_str(), -1, &rect, alignment);
    SelectObject(dc, oldFont);
    DeleteObject(font);
}

void drawFlower(HDC dc, int x, int y, int size, COLORREF petalColor) {
    line(dc, x, y, x, y + size * 2, color(62, 133, 73), 4);
    ellipse(dc, x - 3, y + size / 2, x + size / 2, y + size, color(88, 166, 92));
    for (int petal = 0; petal < 6; ++petal) {
        const double angle = petal * 3.14159265358979323846 / 3.0;
        const int px = x + static_cast<int>(std::cos(angle) * size * .48);
        const int py = y + static_cast<int>(std::sin(angle) * size * .48);
        ellipse(dc, px - size / 3, py - size / 3, px + size / 3, py + size / 3, petalColor);
    }
    ellipse(dc, x - size / 5, y - size / 5, x + size / 5, y + size / 5, color(255, 207, 78));
}

void drawFace(HDC dc, int x, int y, int radius, COLORREF fur,
              const std::wstring& expression, bool dragon = false) {
    ellipse(dc, x - radius, y - radius, x + radius, y + radius, fur, color(255, 255, 255), 3);
    if (dragon) {
        POINT leftHorn[]{{x - radius / 2, y - radius / 2},
                         {x - radius / 2, y - radius - 20}, {x - radius / 8, y - radius / 2}};
        POINT rightHorn[]{{x + radius / 2, y - radius / 2},
                          {x + radius / 2, y - radius - 20}, {x + radius / 8, y - radius / 2}};
        HBRUSH hornBrush = CreateSolidBrush(color(255, 220, 131));
        HGDIOBJ oldBrush = SelectObject(dc, hornBrush);
        Polygon(dc, leftHorn, 3);
        Polygon(dc, rightHorn, 3);
        SelectObject(dc, oldBrush);
        DeleteObject(hornBrush);
    } else {
        ellipse(dc, x - radius * 3 / 4, y - radius * 3 / 4, x - radius / 3, y,
                color(255, 255, 255));
        ellipse(dc, x + radius / 3, y - radius * 3 / 4, x + radius * 3 / 4, y,
                color(255, 255, 255));
    }
    ellipse(dc, x - radius / 3, y - radius / 5, x - radius / 6, y + radius / 12,
            color(48, 57, 54));
    ellipse(dc, x + radius / 6, y - radius / 5, x + radius / 3, y + radius / 12,
            color(48, 57, 54));
    if (expression == L"︵") {
        line(dc, x - radius / 2, y - radius / 3, x - radius / 8, y - radius / 2,
             color(84, 63, 55), 3);
        line(dc, x + radius / 8, y - radius / 2, x + radius / 2, y - radius / 3,
             color(84, 63, 55), 3);
    } else if (expression == L"﹏") {
        line(dc, x - radius / 2, y - radius / 2, x - radius / 8, y - radius / 3,
             color(84, 63, 55), 3);
        line(dc, x + radius / 8, y - radius / 3, x + radius / 2, y - radius / 2,
             color(84, 63, 55), 3);
    }
    text(dc, expression, x - radius / 2, y, x + radius / 2, y + radius / 2,
         radius / 2, color(93, 67, 65));
}

void drawTree(HDC dc, int x, int y) {
    fill(dc, x - 13, y, x + 13, y + 88, color(133, 91, 57));
    ellipse(dc, x - 64, y - 59, x + 25, y + 22, color(90, 170, 98));
    ellipse(dc, x - 18, y - 83, x + 69, y + 11, color(112, 190, 105));
    ellipse(dc, x - 39, y - 38, x + 39, y + 40, color(98, 178, 98));
    ellipse(dc, x - 22, y - 19, x - 14, y - 10, color(43, 73, 50));
    ellipse(dc, x + 12, y - 19, x + 20, y - 10, color(43, 73, 50));
    line(dc, x - 10, y + 1, x, y + 5, color(83, 66, 55), 2);
    line(dc, x, y + 5, x + 10, y + 1, color(83, 66, 55), 2);
}

void drawButterfly(HDC dc, int x, int y) {
    ellipse(dc, x - 26, y - 14, x - 2, y + 12, color(246, 148, 174));
    ellipse(dc, x + 2, y - 14, x + 26, y + 12, color(186, 154, 235));
    line(dc, x, y - 13, x, y + 14, color(85, 83, 104), 4);
    line(dc, x, y - 12, x - 7, y - 22, color(85, 83, 104), 2);
    line(dc, x, y - 12, x + 7, y - 22, color(85, 83, 104), 2);
}

void drawGarden(HDC dc, int width, int height) {
    fill(dc, 0, 0, width, height, color(190, 232, 246));
    fill(dc, 0, 245, width, height, color(175, 218, 143));

    HBRUSH skyBrush = CreateSolidBrush(color(190, 232, 246));
    HGDIOBJ oldBrush = SelectObject(dc, skyBrush);
    const std::array<COLORREF, 6> rainbowColors{{
        color(240, 112, 126), color(255, 177, 96), color(255, 218, 100),
        color(131, 198, 131), color(113, 177, 232), color(174, 143, 221)
    }};
    for (std::size_t i = 0; i < rainbowColors.size(); ++i) {
        HPEN pen = CreatePen(PS_SOLID, 11, rainbowColors[i]);
        HGDIOBJ oldPen = SelectObject(dc, pen);
        SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
        Arc(dc, width / 2 - 178 + static_cast<int>(i) * 10,
            24 + static_cast<int>(i) * 10, width / 2 + 178 - static_cast<int>(i) * 10,
            316 - static_cast<int>(i) * 10, width / 2 - 170, 260,
            width / 2 + 170, 260);
        SelectObject(dc, oldPen);
        DeleteObject(pen);
    }
    SelectObject(dc, oldBrush);
    DeleteObject(skyBrush);

    ellipse(dc, 70, 72, 165, 120, color(255, 255, 255));
    ellipse(dc, 105, 54, 190, 117, color(255, 255, 255));
    ellipse(dc, width - 206, 102, width - 120, 145, color(255, 255, 255));
    ellipse(dc, width - 168, 82, width - 90, 142, color(255, 255, 255));
    ellipse(dc, width - 150, 45, width - 76, 119, color(255, 221, 105));

    ellipse(dc, 30, 231, width - 30, height + 92, color(169, 215, 139),
            color(169, 215, 139));
    ellipse(dc, 355, 250, width - 85, height + 105, color(184, 224, 151),
            color(184, 224, 151));
    drawTree(dc, 104, 270);
    drawTree(dc, width - 96, 275);

    drawFace(dc, width / 2, 204 + animationTick % 3, 58, color(115, 199, 123), L"◡", true);
    ellipse(dc, width / 2 - 69, 220, width / 2 - 23, 256, color(113, 179, 230));
    ellipse(dc, width / 2 + 22, 220, width / 2 + 68, 256, color(113, 179, 230));
    text(dc, L"Draco", width / 2 - 65, 260, width / 2 + 65, 290, 19,
         color(43, 96, 63), DT_CENTER | DT_VCENTER, true);

    const std::array<COLORREF, 5> petals{{
        color(255, 130, 151), color(255, 220, 108), color(182, 151, 234),
        color(255, 255, 240), color(246, 169, 109)
    }};
    for (int i = 0; i < 10; ++i) {
        const int x = 55 + i * (width - 110) / 9;
        const int y = 276 + (i % 3) * 12;
        const COLORREF petal = i < starCount
            ? petals[static_cast<std::size_t>(i) % petals.size()]
            : color(194, 205, 181);
        drawFlower(dc, x, y, 17 + (i % 2) * 5, petal);
        if (i < starCount) {
            text(dc, L"★", x - 15, y - 35, x + 15, y - 7, 19,
                 color(255, 190, 57), DT_CENTER | DT_VCENTER, true);
        }
    }
    drawButterfly(dc, width / 4 + animationTick % 18, 307);
    drawButterfly(dc, width * 3 / 4 - animationTick % 18, 327);
}

void drawButton(HDC dc, RECT rect, const std::wstring& label, bool primary, bool active = false) {
    const COLORREF background = primary ? color(55, 112, 80) : color(245, 250, 232);
    const COLORREF edge = active ? color(244, 184, 74) : color(255, 255, 255);
    roundedBox(dc, rect.left, rect.top, rect.right, rect.bottom, background, edge, 18);
    text(dc, label, rect.left + 8, rect.top + 5, rect.right - 8, rect.bottom - 5,
         17, primary ? color(255, 255, 255) : color(52, 94, 69),
         DT_CENTER | DT_VCENTER | DT_WORDBREAK, true);
}

RECT startButtonRect() { return RECT{430, 734, 670, 790}; }
RECT calmButtonRect() { return RECT{760, 712, 1030, 766}; }
RECT submitButtonRect() { return RECT{430, 714, 670, 770}; }
RECT continueButtonRect() { return RECT{430, 714, 670, 770}; }

void drawCharacterCard(HDC dc, int width) {
    const int x = width / 2;
    const std::wstring expression = currentActivity == 1 ? L"︵" :
                                    currentActivity == 2 ? L"﹏" :
                                    currentActivity == 3 ? L"︵" : L"◡";
    const COLORREF fur = currentActivity == 7 ? color(115, 199, 123) : color(248, 216, 167);
    drawFace(dc, x, 409, 50, fur, expression, currentActivity == 7);
    if (currentActivity == 2) {
        drawFace(dc, x - 105, 409, 35, color(190, 151, 115), L"﹏");
        drawFace(dc, x + 105, 409, 35, color(147, 205, 155), L"○");
    }
    const std::wstring name = currentActivity == 0 ? L"Lila" :
                              currentActivity == 1 ? L"El conejito" :
                              currentActivity == 2 ? L"Osito y ranita" :
                              currentActivity == 6 ? L"Nina" :
                              currentActivity == 7 ? L"Draco" : L"Un amigo del jardín";
    text(dc, name, x - 160, 457, x + 160, 482, 17, color(65, 103, 76),
         DT_CENTER | DT_VCENTER, true);
}

void drawCalmOverlay(HDC dc, int width, int height) {
    fill(dc, 0, 0, width, height, color(111, 148, 128));
    roundedBox(dc, width / 2 - 280, height / 2 - 250, width / 2 + 280,
               height / 2 + 250, color(255, 253, 243), color(255, 255, 255), 30);
    text(dc, L"RINCÓN DE LA CALMA", width / 2 - 240, height / 2 - 205,
         width / 2 + 240, height / 2 - 155, 29, color(55, 107, 85),
         DT_CENTER | DT_VCENTER, true);
    text(dc, L"Toma aire suavemente cuando el círculo crece y suéltalo despacito "
             L"cuando se hace pequeño. Puedes parar cuando quieras.",
         width / 2 - 220, height / 2 - 145, width / 2 + 220, height / 2 - 72,
         19, color(83, 111, 95));
    const int radius = breathingPhase == 0 ? 73 : 52;
    ellipse(dc, width / 2 - radius, height / 2 - 10 - radius,
            width / 2 + radius, height / 2 - 10 + radius,
            color(197, 234, 192), color(142, 197, 145), 4);
    text(dc, breathingPhase == 0 ? L"Toma aire" : L"Suelta despacito",
         width / 2 - 120, height / 2 - 35, width / 2 + 120, height / 2 + 15,
         20, color(55, 107, 85), DT_CENTER | DT_VCENTER, true);
    RECT closeRect{width / 2 - 115, height / 2 + 160, width / 2 + 115, height / 2 + 216};
    drawButton(dc, closeRect, L"Volver al jardín", true);
}

void drawScene(HDC dc, int width, int height) {
    drawGarden(dc, width, height);
    RECT card{80, 322, width - 80, height - 35};
    roundedBox(dc, card.left, card.top, card.right, card.bottom,
               color(255, 253, 243), color(255, 255, 255), 26);

    text(dc, L"EL JARDÍN DE LAS EMOCIONES", 120, 22, width - 120, 64,
         30, color(47, 105, 78), DT_CENTER | DT_VCENTER, true);

    if (screen == Screen::Welcome) {
        text(dc, L"¡Hola, explorador de emociones!", 145, 495, width - 145, 542,
             27, color(46, 91, 68), DT_CENTER | DT_VCENTER, true);
        text(dc, L"Soy Draco. En este jardín, las flores cambian de color según cómo nos sentimos. "
                 L"¿Me ayudas a descubrir emociones y acompañar a mis amigos?",
             160, 547, width - 160, 619, 19, color(89, 115, 97));
        drawButton(dc, startButtonRect(), L"¡Vamos juntos!", true);
        text(dc, L"Todas las emociones tienen espacio. 💚", 220, 682, width - 220, 716,
             16, color(91, 116, 97));
    } else if (screen == Screen::Finished) {
        text(dc, L"¡EL JARDÍN SE ILUMINÓ!", 145, 435, width - 145, 491,
             31, color(47, 105, 78), DT_CENTER | DT_VCENTER, true);
        text(dc, L"¡QUÉ BUEN AMIGO ERES! Draco celebra con todos sus amigos. 🌈",
             155, 493, width - 155, 551, 22, color(89, 115, 97));
        text(dc, L"Gracias por escuchar, pensar y acompañar con cariño. "
                 L"Cada emoción merece ser recibida. 💐",
             155, 551, width - 155, 620, 18, color(89, 115, 97));
        drawButton(dc, startButtonRect(), L"Jugar otra vez", true);
    } else {
        const Activity& activity = activities[currentActivity];
        const std::wstring progress = L"NIVEL " + std::to_wstring(activity.level) +
            L"  ·  " + wide(activity.section) + L"                  Actividad " +
            std::to_wstring(currentActivity + 1) + L" de 9";
        text(dc, progress, 125, 340, width - 125, 370, 16,
             color(86, 115, 96), DT_CENTER | DT_VCENTER, true);
        text(dc, wide(activity.prompt), 145, 369, width - 145, 423, 22,
             color(46, 78, 61), DT_CENTER | DT_VCENTER | DT_WORDBREAK, true);
        text(dc, wide(activity.story), 155, 425, width - 155, 478, 16,
             color(91, 115, 98), DT_CENTER | DT_VCENTER | DT_WORDBREAK);
        drawCharacterCard(dc, width);

        const int gap = 16;
        const int left = 126;
        const int right = width - 126;
        const int middle = width / 2;
        const int top = 493;
        const int rowHeight = 54;
        for (std::size_t i = 0; i < activity.options.size(); ++i) {
            const int row = static_cast<int>(i / 2);
            const int column = static_cast<int>(i % 2);
            RECT optionRect{
                column == 0 ? left : middle + gap / 2,
                top + row * (rowHeight + 10),
                column == 0 ? middle - gap / 2 : right,
                top + row * (rowHeight + 10) + rowHeight
            };
            const bool active = selectedOptions[i];
            drawButton(dc, optionRect, wide(activity.options[i].text), false, active);
            if (active) {
                ellipse(dc, optionRect.left + 14, optionRect.top + 18,
                        optionRect.left + 31, optionRect.top + 35, color(106, 166, 112));
            }
        }
        if (answered) {
            text(dc, L"Gracias por compartir tu idea. ¡VAMOS A PENSAR JUNTOS! ⭐",
                 150, 625, width - 150, 665, 17, color(62, 116, 74),
                 DT_CENTER | DT_VCENTER, true);
            drawButton(dc, continueButtonRect(), L"Siguiente", true);
        } else {
            text(dc, L"Puedes elegir una o varias respuestas. No hay una sola forma de sentirse.",
                 150, 625, width - 150, 665, 16, color(91, 115, 98));
            drawButton(dc, submitButtonRect(), L"Compartir mi idea", true);
        }
        drawButton(dc, calmButtonRect(), L"Rincón de la calma", false);
        text(dc, L"🌼", 90, 704, 150, 746, 24, color(91, 115, 98));
    }
    if (calmOpen) drawCalmOverlay(dc, width, height);
}

void resetActivity() {
    std::fill(std::begin(selectedOptions), std::end(selectedOptions), false);
    answered = false;
}

void handleClick(HWND window, int x, int y, int width, int height) {
    if (calmOpen) {
        RECT closeRect{width / 2 - 115, height / 2 + 160, width / 2 + 115, height / 2 + 216};
        if (x >= closeRect.left && x <= closeRect.right &&
            y >= closeRect.top && y <= closeRect.bottom) {
            calmOpen = false;
            InvalidateRect(window, nullptr, FALSE);
        }
        return;
    }

    if (screen == Screen::Welcome || screen == Screen::Finished) {
        const RECT button = startButtonRect();
        if (x >= button.left && x <= button.right && y >= button.top && y <= button.bottom) {
            screen = Screen::Playing;
            currentActivity = 0;
            starCount = 0;
            resetActivity();
            InvalidateRect(window, nullptr, FALSE);
        }
        return;
    }

    const RECT calm = calmButtonRect();
    if (x >= calm.left && x <= calm.right && y >= calm.top && y <= calm.bottom) {
        calmOpen = true;
        InvalidateRect(window, nullptr, FALSE);
        return;
    }

    const Activity& activity = activities[currentActivity];
    if (!answered) {
        const int left = 126;
        const int right = width - 126;
        const int middle = width / 2;
        for (std::size_t i = 0; i < activity.options.size(); ++i) {
            const int row = static_cast<int>(i / 2);
            const int column = static_cast<int>(i % 2);
            RECT optionRect{
                column == 0 ? left : middle + 8,
                493 + row * 64,
                column == 0 ? middle - 8 : right,
                547 + row * 64
            };
            if (x >= optionRect.left && x <= optionRect.right &&
                y >= optionRect.top && y <= optionRect.bottom) {
                selectedOptions[i] = !selectedOptions[i];
                InvalidateRect(window, nullptr, FALSE);
                return;
            }
        }
        const RECT submit = submitButtonRect();
        if (x >= submit.left && x <= submit.right && y >= submit.top && y <= submit.bottom) {
            if (std::any_of(std::begin(selectedOptions), std::end(selectedOptions),
                            [](bool selected) { return selected; })) {
                answered = true;
                ++starCount;
                MessageBeep(MB_OK);
                InvalidateRect(window, nullptr, FALSE);
            }
        }
        return;
    }

    const RECT next = continueButtonRect();
    if (x >= next.left && x <= next.right && y >= next.top && y <= next.bottom) {
        ++currentActivity;
        resetActivity();
        if (currentActivity >= activities.size()) screen = Screen::Finished;
        InvalidateRect(window, nullptr, FALSE);
    }
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        SetTimer(window, 1, 900, nullptr);
        return 0;
    case WM_TIMER:
        ++animationTick;
        breathingPhase = (breathingPhase + 1) % 2;
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_LBUTTONDOWN: {
        RECT rect{};
        GetClientRect(window, &rect);
        handleClick(window, LOWORD(lParam), HIWORD(lParam), rect.right, rect.bottom);
        return 0;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE && calmOpen) {
            calmOpen = false;
            InvalidateRect(window, nullptr, FALSE);
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT rect{};
        GetClientRect(window, &rect);
        HDC buffer = CreateCompatibleDC(dc);
        HBITMAP bitmap = CreateCompatibleBitmap(dc, rect.right, rect.bottom);
        HGDIOBJ oldBitmap = SelectObject(buffer, bitmap);
        drawScene(buffer, rect.right, rect.bottom);
        BitBlt(dc, 0, 0, rect.right, rect.bottom, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(window, 1);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}
}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    SetProcessDPIAware();
    const wchar_t CLASS_NAME[] = L"JardinDeLasEmociones";
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = CreateSolidBrush(color(190, 232, 246));
    windowClass.lpszClassName = CLASS_NAME;
    if (!RegisterClassW(&windowClass)) return 1;

    RECT bounds{0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE);
    HWND window = CreateWindowExW(
        0, CLASS_NAME, L"EL JARDÍN DE LAS EMOCIONES", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, bounds.right - bounds.left, bounds.bottom - bounds.top,
        nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}

#else

void showGarden(int stars) {
    std::cout << "\n  ☁️       🌈       ☁️\n"
              << "          🐉 Draco\n  🌳  ";
    for (int i = 0; i < 9; ++i) {
        std::cout << (i < stars ? "🌟" : "🌱") << ' ';
    }
    std::cout << "🌳\n  🌼 🌷 🌻 🦋 🌸 🌼 🐦 🌷\n\n";
}

void calmCorner() {
    std::cout << "\n🍃 RINCÓN DE LA CALMA 🍃\n"
              << "Sigue el círculo: toma aire suavemente cuando crece y suéltalo despacito "
                 "cuando se hace pequeño. Puedes parar cuando quieras.\n\n";
    for (int cycle = 0; cycle < 3; ++cycle) {
        std::cout << "        (  TOMA AIRE  )\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(650));
        std::cout << "          ( SUELTA )\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(650));
    }
    std::cout << "Muy bien. Puedes volver cuando lo necesites. 🐦 🔔\n\n";
}

bool readSelection(const Activity& activity, std::vector<std::size_t>& selection) {
    while (true) {
        std::cout << "\nElige una o varias ideas escribiendo sus números separados por comas.\n"
                  << "También puedes escribir C para visitar el Rincón de la calma.\n> ";
        std::string input;
        if (!std::getline(std::cin, input)) return false;

        if (input == "c" || input == "C") {
            calmCorner();
            continue;
        }

        std::vector<std::size_t> parsed;
        std::stringstream inputStream(input);
        std::string token;
        bool valid = true;
        while (std::getline(inputStream, token, ',')) {
            std::stringstream tokenStream(token);
            std::size_t choice = 0;
            char extra = '\0';
            if (!(tokenStream >> choice) || (tokenStream >> extra) ||
                choice == 0 || choice > activity.options.size()) {
                valid = false;
                break;
            }
            if (std::find(parsed.begin(), parsed.end(), choice - 1) == parsed.end()) {
                parsed.push_back(choice - 1);
            }
        }

        if (valid && !parsed.empty()) {
            selection = std::move(parsed);
            return true;
        }
        std::cout << "Puedes tomarte tu tiempo. Prueba con uno de los números de la lista. 💚\n";
    }
}

void playActivity(const Activity& activity, std::size_t index, int& stars) {
    std::cout << "\n============================================================\n"
              << "NIVEL " << activity.level << " · " << activity.section
              << "                    Actividad " << index + 1 << " de 9\n\n"
              << activity.prompt << "\n"
              << activity.story << "\n\n"
              << "  " << activity.characters << "\n\n";
    for (std::size_t i = 0; i < activity.options.size(); ++i) {
        std::cout << "  " << i + 1 << ") " << activity.options[i].emoji << " "
                  << activity.options[i].text << '\n';
    }

    std::vector<std::size_t> selection;
    if (!readSelection(activity, selection)) return;

    std::cout << "\nElegiste: ";
    for (std::size_t i = 0; i < selection.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << activity.options[selection[i]].text;
    }
    std::cout << ". Gracias por compartir tus ideas. Cada persona puede sentirse "
                 "de manera diferente.\n"
              << "¡QUÉ BUEN AMIGO ERES! ¡VAMOS A PENSAR JUNTOS! "
              << activity.flower << " ⭐\n";
    std::cout << '\a';
    ++stars;
    showGarden(stars);
}

int main() {
    std::cout << "============================================================\n"
              << "              EL JARDÍN DE LAS EMOCIONES\n"
              << "============================================================\n"
              << "¡Hola! Soy Draco, un pequeño dragón mágico. 🐉\n"
              << "En este jardín, todas las emociones tienen espacio.\n"
              << "No hay una sola forma de sentirse: puedes elegir varias ideas.\n"
              << "¡Vamos a descubrir emociones y acompañar a nuestros amigos!\n";
    showGarden(0);

    int stars = 0;
    for (std::size_t i = 0; i < activities.size(); ++i) {
        playActivity(activities[i], i, stars);
        if (!std::cin) return 0;
    }

    std::cout << "\n============================================================\n"
              << "🌈 ¡EL JARDÍN SE ILUMINÓ! 🌈\n"
              << "🐉 Draco celebra con todos sus amigos. 🐻 🐰 🦊 🐸 🦋\n"
              << "¡QUÉ BUEN AMIGO ERES! Gracias por escuchar y acompañar.\n\n";
    showGarden(9);
    std::cout << "¡Hasta la próxima aventura! 💚\n";
    return 0;
}

#endif
