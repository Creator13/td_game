#pragma once
#include "math/func.h"
#include "math/vec4.h"

namespace core
{
    struct SrgbColor;

    /// Represents a color in linear color space.
    struct Color
    {
        float r, g, b, a;

        constexpr Color() : r(0), g(0), b(0), a(0) { }
        constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) { }
        constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) { }

        [[nodiscard]] static constexpr Color fromSrgb(float r, float g, float b, float a);
        [[nodiscard]] static constexpr Color fromSrgb(math::vec4 rgba);
        [[nodiscard]] static constexpr Color fromSrgb(const SrgbColor& srgbColor);
        [[nodiscard]] static constexpr Color fromSrgb(float r, float g, float b);
        [[nodiscard]] static constexpr Color fromSrgb(math::vec3 rgb);

        [[nodiscard]] constexpr math::vec3 rgb() const { return math::vec3(r, g, b); }

        constexpr explicit operator math::vec4() const { return math::vec4(r, g, b, a); }

        // ReSharper disable CppInconsistentNaming
        static const Color black, white, red, green, blue, yellow, cyan, magenta;
        static const Color gray, grey, silver, darkGray, dimGray, lightGray, gainsboro, whiteSmoke;
        static const Color darkRed, crimson, fireBrick, indianRed, lightCoral, salmon, darkSalmon, lightSalmon;
        static const Color orange, darkOrange, orangeRed, tomato, coral, gold, lightYellow, lemonChiffon;
        static const Color lightGoldenrodYellow, papayaWhip, moccasin, peachPuff, paleGoldenrod, khaki, darkKhaki;
        static const Color lime, limeGreen, lawnGreen, chartreuse, greenYellow, springGreen, mediumSpringGreen;
        static const Color lightGreen, paleGreen, darkSeaGreen, mediumSeaGreen, seaGreen, forestGreen, darkGreen;
        static const Color yellowGreen, oliveDrab, olive, darkOliveGreen;
        static const Color aqua, aquamarine, mediumAquamarine, lightCyan, paleTurquoise, turquoise, mediumTurquoise;
        static const Color darkTurquoise, lightSeaGreen, cadetBlue, darkCyan, teal;
        static const Color lightSteelBlue, powderBlue, lightBlue, skyBlue, lightSkyBlue, deepSkyBlue, dodgerBlue;
        static const Color cornflowerBlue, steelBlue, royalBlue, mediumBlue, darkBlue, navy, midnightBlue;
        static const Color lavender, thistle, plum, violet, orchid, fuchsia, mediumOrchid, mediumPurple;
        static const Color blueViolet, darkViolet, darkOrchid, darkMagenta, purple, rebeccaPurple, indigo;
        static const Color slateBlue, darkSlateBlue, mediumSlateBlue;
        static const Color pink, lightPink, hotPink, deepPink, mediumVioletRed, paleVioletRed;
        static const Color cornsilk, blanchedAlmond, bisque, navajoWhite, wheat, burlyWood, tan, rosyBrown;
        static const Color sandyBrown, goldenrod, darkGoldenrod, peru, chocolate, saddleBrown, sienna, brown, maroon;
        static const Color snow, honeydew, mintCream, azure, aliceBlue, ghostWhite, seashell, beige, oldLace;
        static const Color floralWhite, ivory, antiqueWhite, linen, lavenderBlush, mistyRose;
        static const Color slateGray, lightSlateGray, darkSlateGray;
        static const Color gray1, gray2, gray3, gray4, gray5, gray6, gray7, gray8, gray9;
        // ReSharper enable CppInconsistentNaming
    };

    // @formatter:off
    // ReSharper disable CppIdenticalOperandsInBinaryExpression
    // ReSharper disable CppInconsistentNaming
    inline const Color Color::black                = fromSrgb(  0.0f / 255.0f,   0.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::white                = fromSrgb(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::red                  = fromSrgb(255.0f / 255.0f,   0.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::green                = fromSrgb(  0.0f / 255.0f, 128.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::blue                 = fromSrgb(  0.0f / 255.0f,   0.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::yellow               = fromSrgb(255.0f / 255.0f, 255.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::cyan                 = fromSrgb(  0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::magenta              = fromSrgb(255.0f / 255.0f,   0.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::gray                 = fromSrgb(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f);
    inline const Color Color::grey                 = fromSrgb(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f);
    inline const Color Color::silver               = fromSrgb(192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f);
    inline const Color Color::darkGray             = fromSrgb(169.0f / 255.0f, 169.0f / 255.0f, 169.0f / 255.0f);
    inline const Color Color::dimGray              = fromSrgb(105.0f / 255.0f, 105.0f / 255.0f, 105.0f / 255.0f);
    inline const Color Color::lightGray            = fromSrgb(211.0f / 255.0f, 211.0f / 255.0f, 211.0f / 255.0f);
    inline const Color Color::gainsboro            = fromSrgb(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f);
    inline const Color Color::whiteSmoke           = fromSrgb(245.0f / 255.0f, 245.0f / 255.0f, 245.0f / 255.0f);
    inline const Color Color::darkRed              = fromSrgb(139.0f / 255.0f,   0.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::crimson              = fromSrgb(220.0f / 255.0f,  20.0f / 255.0f,  60.0f / 255.0f);
    inline const Color Color::fireBrick            = fromSrgb(178.0f / 255.0f,  34.0f / 255.0f,  34.0f / 255.0f);
    inline const Color Color::indianRed            = fromSrgb(205.0f / 255.0f,  92.0f / 255.0f,  92.0f / 255.0f);
    inline const Color Color::lightCoral           = fromSrgb(240.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f);
    inline const Color Color::salmon               = fromSrgb(250.0f / 255.0f, 128.0f / 255.0f, 114.0f / 255.0f);
    inline const Color Color::darkSalmon           = fromSrgb(233.0f / 255.0f, 150.0f / 255.0f, 122.0f / 255.0f);
    inline const Color Color::lightSalmon          = fromSrgb(255.0f / 255.0f, 160.0f / 255.0f, 122.0f / 255.0f);
    inline const Color Color::orange               = fromSrgb(255.0f / 255.0f, 165.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::darkOrange           = fromSrgb(255.0f / 255.0f, 140.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::orangeRed            = fromSrgb(255.0f / 255.0f,  69.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::tomato               = fromSrgb(255.0f / 255.0f,  99.0f / 255.0f,  71.0f / 255.0f);
    inline const Color Color::coral                = fromSrgb(255.0f / 255.0f, 127.0f / 255.0f,  80.0f / 255.0f);
    inline const Color Color::gold                 = fromSrgb(255.0f / 255.0f, 215.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::lightYellow          = fromSrgb(255.0f / 255.0f, 255.0f / 255.0f, 224.0f / 255.0f);
    inline const Color Color::lemonChiffon         = fromSrgb(255.0f / 255.0f, 250.0f / 255.0f, 205.0f / 255.0f);
    inline const Color Color::lightGoldenrodYellow = fromSrgb(250.0f / 255.0f, 250.0f / 255.0f, 210.0f / 255.0f);
    inline const Color Color::papayaWhip           = fromSrgb(255.0f / 255.0f, 239.0f / 255.0f, 213.0f / 255.0f);
    inline const Color Color::moccasin             = fromSrgb(255.0f / 255.0f, 228.0f / 255.0f, 181.0f / 255.0f);
    inline const Color Color::peachPuff            = fromSrgb(255.0f / 255.0f, 218.0f / 255.0f, 185.0f / 255.0f);
    inline const Color Color::paleGoldenrod        = fromSrgb(238.0f / 255.0f, 232.0f / 255.0f, 170.0f / 255.0f);
    inline const Color Color::khaki                = fromSrgb(240.0f / 255.0f, 230.0f / 255.0f, 140.0f / 255.0f);
    inline const Color Color::darkKhaki            = fromSrgb(189.0f / 255.0f, 183.0f / 255.0f, 107.0f / 255.0f);
    inline const Color Color::lime                 = fromSrgb(  0.0f / 255.0f, 255.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::limeGreen            = fromSrgb( 50.0f / 255.0f, 205.0f / 255.0f,  50.0f / 255.0f);
    inline const Color Color::lawnGreen            = fromSrgb(124.0f / 255.0f, 252.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::chartreuse           = fromSrgb(127.0f / 255.0f, 255.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::greenYellow          = fromSrgb(173.0f / 255.0f, 255.0f / 255.0f,  47.0f / 255.0f);
    inline const Color Color::springGreen          = fromSrgb(  0.0f / 255.0f, 255.0f / 255.0f, 127.0f / 255.0f);
    inline const Color Color::mediumSpringGreen    = fromSrgb(  0.0f / 255.0f, 250.0f / 255.0f, 154.0f / 255.0f);
    inline const Color Color::lightGreen           = fromSrgb(144.0f / 255.0f, 238.0f / 255.0f, 144.0f / 255.0f);
    inline const Color Color::paleGreen            = fromSrgb(152.0f / 255.0f, 251.0f / 255.0f, 152.0f / 255.0f);
    inline const Color Color::darkSeaGreen         = fromSrgb(143.0f / 255.0f, 188.0f / 255.0f, 143.0f / 255.0f);
    inline const Color Color::mediumSeaGreen       = fromSrgb( 60.0f / 255.0f, 179.0f / 255.0f, 113.0f / 255.0f);
    inline const Color Color::seaGreen             = fromSrgb( 46.0f / 255.0f, 139.0f / 255.0f,  87.0f / 255.0f);
    inline const Color Color::forestGreen          = fromSrgb( 34.0f / 255.0f, 139.0f / 255.0f,  34.0f / 255.0f);
    inline const Color Color::darkGreen            = fromSrgb(  0.0f / 255.0f, 100.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::yellowGreen          = fromSrgb(154.0f / 255.0f, 205.0f / 255.0f,  50.0f / 255.0f);
    inline const Color Color::oliveDrab            = fromSrgb(107.0f / 255.0f, 142.0f / 255.0f,  35.0f / 255.0f);
    inline const Color Color::olive                = fromSrgb(128.0f / 255.0f, 128.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::darkOliveGreen       = fromSrgb( 85.0f / 255.0f, 107.0f / 255.0f,  47.0f / 255.0f);
    inline const Color Color::aqua                 = fromSrgb(  0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::aquamarine           = fromSrgb(127.0f / 255.0f, 255.0f / 255.0f, 212.0f / 255.0f);
    inline const Color Color::mediumAquamarine     = fromSrgb(102.0f / 255.0f, 205.0f / 255.0f, 170.0f / 255.0f);
    inline const Color Color::lightCyan            = fromSrgb(224.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::paleTurquoise        = fromSrgb(175.0f / 255.0f, 238.0f / 255.0f, 238.0f / 255.0f);
    inline const Color Color::turquoise            = fromSrgb( 64.0f / 255.0f, 224.0f / 255.0f, 208.0f / 255.0f);
    inline const Color Color::mediumTurquoise      = fromSrgb( 72.0f / 255.0f, 209.0f / 255.0f, 204.0f / 255.0f);
    inline const Color Color::darkTurquoise        = fromSrgb(  0.0f / 255.0f, 206.0f / 255.0f, 209.0f / 255.0f);
    inline const Color Color::lightSeaGreen        = fromSrgb( 32.0f / 255.0f, 178.0f / 255.0f, 170.0f / 255.0f);
    inline const Color Color::cadetBlue            = fromSrgb( 95.0f / 255.0f, 158.0f / 255.0f, 160.0f / 255.0f);
    inline const Color Color::darkCyan             = fromSrgb(  0.0f / 255.0f, 139.0f / 255.0f, 139.0f / 255.0f);
    inline const Color Color::teal                 = fromSrgb(  0.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f);
    inline const Color Color::lightSteelBlue       = fromSrgb(176.0f / 255.0f, 196.0f / 255.0f, 222.0f / 255.0f);
    inline const Color Color::powderBlue           = fromSrgb(176.0f / 255.0f, 224.0f / 255.0f, 230.0f / 255.0f);
    inline const Color Color::lightBlue            = fromSrgb(173.0f / 255.0f, 216.0f / 255.0f, 230.0f / 255.0f);
    inline const Color Color::skyBlue              = fromSrgb(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f);
    inline const Color Color::lightSkyBlue         = fromSrgb(135.0f / 255.0f, 206.0f / 255.0f, 250.0f / 255.0f);
    inline const Color Color::deepSkyBlue          = fromSrgb(  0.0f / 255.0f, 191.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::dodgerBlue           = fromSrgb( 30.0f / 255.0f, 144.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::cornflowerBlue       = fromSrgb(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f);
    inline const Color Color::steelBlue            = fromSrgb( 70.0f / 255.0f, 130.0f / 255.0f, 180.0f / 255.0f);
    inline const Color Color::royalBlue            = fromSrgb( 65.0f / 255.0f, 105.0f / 255.0f, 225.0f / 255.0f);
    inline const Color Color::mediumBlue           = fromSrgb(  0.0f / 255.0f,   0.0f / 255.0f, 205.0f / 255.0f);
    inline const Color Color::darkBlue             = fromSrgb(  0.0f / 255.0f,   0.0f / 255.0f, 139.0f / 255.0f);
    inline const Color Color::navy                 = fromSrgb(  0.0f / 255.0f,   0.0f / 255.0f, 128.0f / 255.0f);
    inline const Color Color::midnightBlue         = fromSrgb( 25.0f / 255.0f,  25.0f / 255.0f, 112.0f / 255.0f);
    inline const Color Color::lavender             = fromSrgb(230.0f / 255.0f, 230.0f / 255.0f, 250.0f / 255.0f);
    inline const Color Color::thistle              = fromSrgb(216.0f / 255.0f, 191.0f / 255.0f, 216.0f / 255.0f);
    inline const Color Color::plum                 = fromSrgb(221.0f / 255.0f, 160.0f / 255.0f, 221.0f / 255.0f);
    inline const Color Color::violet               = fromSrgb(238.0f / 255.0f, 130.0f / 255.0f, 238.0f / 255.0f);
    inline const Color Color::orchid               = fromSrgb(218.0f / 255.0f, 112.0f / 255.0f, 214.0f / 255.0f);
    inline const Color Color::fuchsia              = fromSrgb(255.0f / 255.0f,   0.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::mediumOrchid         = fromSrgb(186.0f / 255.0f,  85.0f / 255.0f, 211.0f / 255.0f);
    inline const Color Color::mediumPurple         = fromSrgb(147.0f / 255.0f, 112.0f / 255.0f, 219.0f / 255.0f);
    inline const Color Color::blueViolet           = fromSrgb(138.0f / 255.0f,  43.0f / 255.0f, 226.0f / 255.0f);
    inline const Color Color::darkViolet           = fromSrgb(148.0f / 255.0f,   0.0f / 255.0f, 211.0f / 255.0f);
    inline const Color Color::darkOrchid           = fromSrgb(153.0f / 255.0f,  50.0f / 255.0f, 204.0f / 255.0f);
    inline const Color Color::darkMagenta          = fromSrgb(139.0f / 255.0f,   0.0f / 255.0f, 139.0f / 255.0f);
    inline const Color Color::purple               = fromSrgb(128.0f / 255.0f,   0.0f / 255.0f, 128.0f / 255.0f);
    inline const Color Color::rebeccaPurple        = fromSrgb(102.0f / 255.0f,  51.0f / 255.0f, 153.0f / 255.0f);
    inline const Color Color::indigo               = fromSrgb( 75.0f / 255.0f,   0.0f / 255.0f, 130.0f / 255.0f);
    inline const Color Color::slateBlue            = fromSrgb(106.0f / 255.0f,  90.0f / 255.0f, 205.0f / 255.0f);
    inline const Color Color::darkSlateBlue        = fromSrgb( 72.0f / 255.0f,  61.0f / 255.0f, 139.0f / 255.0f);
    inline const Color Color::mediumSlateBlue      = fromSrgb(123.0f / 255.0f, 104.0f / 255.0f, 238.0f / 255.0f);
    inline const Color Color::pink                 = fromSrgb(255.0f / 255.0f, 192.0f / 255.0f, 203.0f / 255.0f);
    inline const Color Color::lightPink            = fromSrgb(255.0f / 255.0f, 182.0f / 255.0f, 193.0f / 255.0f);
    inline const Color Color::hotPink              = fromSrgb(255.0f / 255.0f, 105.0f / 255.0f, 180.0f / 255.0f);
    inline const Color Color::deepPink             = fromSrgb(255.0f / 255.0f,  20.0f / 255.0f, 147.0f / 255.0f);
    inline const Color Color::mediumVioletRed      = fromSrgb(199.0f / 255.0f,  21.0f / 255.0f, 133.0f / 255.0f);
    inline const Color Color::paleVioletRed        = fromSrgb(219.0f / 255.0f, 112.0f / 255.0f, 147.0f / 255.0f);
    inline const Color Color::cornsilk             = fromSrgb(255.0f / 255.0f, 248.0f / 255.0f, 220.0f / 255.0f);
    inline const Color Color::blanchedAlmond       = fromSrgb(255.0f / 255.0f, 235.0f / 255.0f, 205.0f / 255.0f);
    inline const Color Color::bisque               = fromSrgb(255.0f / 255.0f, 228.0f / 255.0f, 196.0f / 255.0f);
    inline const Color Color::navajoWhite          = fromSrgb(255.0f / 255.0f, 222.0f / 255.0f, 173.0f / 255.0f);
    inline const Color Color::wheat                = fromSrgb(245.0f / 255.0f, 222.0f / 255.0f, 179.0f / 255.0f);
    inline const Color Color::burlyWood            = fromSrgb(222.0f / 255.0f, 184.0f / 255.0f, 135.0f / 255.0f);
    inline const Color Color::tan                  = fromSrgb(210.0f / 255.0f, 180.0f / 255.0f, 140.0f / 255.0f);
    inline const Color Color::rosyBrown            = fromSrgb(188.0f / 255.0f, 143.0f / 255.0f, 143.0f / 255.0f);
    inline const Color Color::sandyBrown           = fromSrgb(244.0f / 255.0f, 164.0f / 255.0f,  96.0f / 255.0f);
    inline const Color Color::goldenrod            = fromSrgb(218.0f / 255.0f, 165.0f / 255.0f,  32.0f / 255.0f);
    inline const Color Color::darkGoldenrod        = fromSrgb(184.0f / 255.0f, 134.0f / 255.0f,  11.0f / 255.0f);
    inline const Color Color::peru                 = fromSrgb(205.0f / 255.0f, 133.0f / 255.0f,  63.0f / 255.0f);
    inline const Color Color::chocolate            = fromSrgb(210.0f / 255.0f, 105.0f / 255.0f,  30.0f / 255.0f);
    inline const Color Color::saddleBrown          = fromSrgb(139.0f / 255.0f,  69.0f / 255.0f,  19.0f / 255.0f);
    inline const Color Color::sienna               = fromSrgb(160.0f / 255.0f,  82.0f / 255.0f,  45.0f / 255.0f);
    inline const Color Color::brown                = fromSrgb(165.0f / 255.0f,  42.0f / 255.0f,  42.0f / 255.0f);
    inline const Color Color::maroon               = fromSrgb(128.0f / 255.0f,   0.0f / 255.0f,   0.0f / 255.0f);
    inline const Color Color::snow                 = fromSrgb(255.0f / 255.0f, 250.0f / 255.0f, 250.0f / 255.0f);
    inline const Color Color::honeydew             = fromSrgb(240.0f / 255.0f, 255.0f / 255.0f, 240.0f / 255.0f);
    inline const Color Color::mintCream            = fromSrgb(245.0f / 255.0f, 255.0f / 255.0f, 250.0f / 255.0f);
    inline const Color Color::azure                = fromSrgb(240.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::aliceBlue            = fromSrgb(240.0f / 255.0f, 248.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::ghostWhite           = fromSrgb(248.0f / 255.0f, 248.0f / 255.0f, 255.0f / 255.0f);
    inline const Color Color::seashell             = fromSrgb(255.0f / 255.0f, 245.0f / 255.0f, 238.0f / 255.0f);
    inline const Color Color::beige                = fromSrgb(245.0f / 255.0f, 245.0f / 255.0f, 220.0f / 255.0f);
    inline const Color Color::oldLace              = fromSrgb(253.0f / 255.0f, 245.0f / 255.0f, 230.0f / 255.0f);
    inline const Color Color::floralWhite          = fromSrgb(255.0f / 255.0f, 250.0f / 255.0f, 240.0f / 255.0f);
    inline const Color Color::ivory                = fromSrgb(255.0f / 255.0f, 255.0f / 255.0f, 240.0f / 255.0f);
    inline const Color Color::antiqueWhite         = fromSrgb(250.0f / 255.0f, 235.0f / 255.0f, 215.0f / 255.0f);
    inline const Color Color::linen                = fromSrgb(250.0f / 255.0f, 240.0f / 255.0f, 230.0f / 255.0f);
    inline const Color Color::lavenderBlush        = fromSrgb(255.0f / 255.0f, 240.0f / 255.0f, 245.0f / 255.0f);
    inline const Color Color::mistyRose            = fromSrgb(255.0f / 255.0f, 228.0f / 255.0f, 225.0f / 255.0f);
    inline const Color Color::slateGray            = fromSrgb(112.0f / 255.0f, 128.0f / 255.0f, 144.0f / 255.0f);
    inline const Color Color::lightSlateGray       = fromSrgb(119.0f / 255.0f, 136.0f / 255.0f, 153.0f / 255.0f);
    inline const Color Color::darkSlateGray        = fromSrgb( 47.0f / 255.0f,  79.0f / 255.0f,  79.0f / 255.0f);
    inline const Color Color::gray1                = fromSrgb(.1f, .1f, .1f, .1f);
    inline const Color Color::gray2                = fromSrgb(.2f, .2f, .2f, .2f);
    inline const Color Color::gray3                = fromSrgb(.3f, .3f, .3f, .3f);
    inline const Color Color::gray4                = fromSrgb(.4f, .4f, .4f, .4f);
    inline const Color Color::gray5                = fromSrgb(.5f, .5f, .5f, .5f);
    inline const Color Color::gray6                = fromSrgb(.6f, .6f, .6f, .6f);
    inline const Color Color::gray7                = fromSrgb(.7f, .7f, .7f, .7f);
    inline const Color Color::gray8                = fromSrgb(.8f, .8f, .8f, .8f);
    inline const Color Color::gray9                = fromSrgb(.9f, .9f, .9f, .9f);
    // ReSharper restore CppInconsistentNaming
    // ReSharper restore CppIdenticalOperandsInBinaryExpression
    // @formatter:on

    /// Represents a color, assumed to be in sRGB space.
    struct SrgbColor
    {
        float r, g, b, a;

        constexpr SrgbColor() : r(0), g(0), b(0), a(0) { }
        constexpr SrgbColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) { }
        constexpr SrgbColor(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) { }

        [[nodiscard]] static constexpr SrgbColor fromLinear(float r, float g, float b, float a);
        [[nodiscard]] static constexpr SrgbColor fromLinear(const Color& color);
        [[nodiscard]] static constexpr SrgbColor fromLinear(float r, float g, float b);
    };

    constexpr float srgbToLinear(float x)
    {
        //  sRGB transform (C++)
        //
        // Copyright (c) 2017 Project Nayuki. (MIT License)
        // https://www.nayuki.io/page/srgb-transform-library

        if (x <= 0.0f) return 0.0f;

        if (x >= 1.0f) return 1.0f;

        if (x < 0.04045f) return x / 12.92f;

        return math::pow((x + 0.055f) / 1.055f, 2.4f);
    }

    constexpr float linearToSrgb(float x)
    {
        //  sRGB transform (C++)
        //
        // Copyright (c) 2017 Project Nayuki. (MIT License)
        // https://www.nayuki.io/page/srgb-transform-library

        if (x <= 0.0f) return 0.0f;

        if (x >= 1.0f) return 1.0f;

        if (x < 0.0031308f) return x * 12.92f;

        return math::pow(x, 1.0f / 2.4f) * 1.055f - 0.055f;
    }

    constexpr Color Color::fromSrgb(float r, float g, float b, float a)
    {
        Color result;
        result.r = srgbToLinear(r);
        result.g = srgbToLinear(g);
        result.b = srgbToLinear(b);
        result.a = a;
        return result;
    }

    constexpr Color Color::fromSrgb(math::vec4 rgba)
    {
        return fromSrgb(rgba.x, rgba.y, rgba.z, rgba.w);
    }

    constexpr Color Color::fromSrgb(const SrgbColor& srgbColor)
    {
        return fromSrgb(srgbColor.r, srgbColor.g, srgbColor.b, srgbColor.a);
    }

    constexpr Color Color::fromSrgb(float r, float g, float b)
    {
        return fromSrgb(r, g, b, 1.0f);
    }

    constexpr Color Color::fromSrgb(math::vec3 rgb)
    {
        return fromSrgb(rgb.x, rgb.y, rgb.z);
    }

    constexpr SrgbColor SrgbColor::fromLinear(float r, float g, float b, float a)
    {
        SrgbColor result;
        result.r = linearToSrgb(r);
        result.g = linearToSrgb(g);
        result.b = linearToSrgb(b);
        result.a = a;
        return result;
    }

    constexpr SrgbColor SrgbColor::fromLinear(const Color& color)
    {
        return fromLinear(color.r, color.g, color.b, color.a);
    }

    constexpr SrgbColor SrgbColor::fromLinear(float r, float g, float b)
    {
        return fromLinear(r, g, b, 1.0f);
    }
}
