// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include <string>
#include <iterator>
#include "BMFont.hpp"
#include "core/Engine.hpp"
#include "assets/Cache.hpp"
#include "utils/Errors.hpp"
#include "utils/Utils.hpp"

namespace ouzel
{
    static inline bool isWhitespace(uint8_t c)
    {
        return c == ' ' || c == '\t';
    }

    static inline bool isNewline(uint8_t c)
    {
        return c == '\r' || c == '\n';
    }

    static inline bool isControlChar(uint8_t c)
    {
        return c <= 0x1F;
    }

    static void skipWhitespaces(const std::vector<uint8_t>& str,
                                std::vector<uint8_t>::const_iterator& iterator)
    {
        for (;;)
        {
            if (iterator == str.end()) break;

            if (isWhitespace(*iterator))
                ++iterator;
            else
                break;
        }
    }

    static void skipLine(const std::vector<uint8_t>& str,
                         std::vector<uint8_t>::const_iterator& iterator)
    {
        for (;;)
        {
            if (iterator == str.end()) break;

            if (isNewline(*iterator))
            {
                ++iterator;
                break;
            }

            ++iterator;
        }
    }

    static std::string parseString(const std::vector<uint8_t>& str,
                                   std::vector<uint8_t>::const_iterator& iterator)
    {
        std::string result;

        if (*iterator == '"')
        {
            ++iterator;

            for (;;)
            {
                if (*iterator == '"' &&
                    (iterator + 1 == str.end() ||
                     isWhitespace(*(iterator + 1)) ||
                     isNewline(*(iterator + 1))))
                {
                    ++iterator;
                    break;
                }
                if (iterator == str.end())
                    throw ParseError("Unterminated string");

                result.push_back(static_cast<char>(*iterator));

                ++iterator;
            }
        }
        else
        {
            for (;;)
            {
                if (iterator == str.end() || isControlChar(*iterator) || isWhitespace(*iterator) || *iterator == '=') break;

                result.push_back(static_cast<char>(*iterator));

                ++iterator;
            }

            if (result.empty())
                throw ParseError("Invalid string");
        }

        return result;
    }

    static std::string parseInt(const std::vector<uint8_t>& str,
                                std::vector<uint8_t>::const_iterator& iterator)
    {
        std::string result;
        uint32_t length = 1;

        if (iterator != str.end() && *iterator == '-')
        {
            result.push_back(static_cast<char>(*iterator));
            ++length;
            ++iterator;
        }

        for (;;)
        {
            if (iterator == str.end() || *iterator < '0' || *iterator > '9') break;

            result.push_back(static_cast<char>(*iterator));

            ++iterator;
        }

        if (result.length() < length)
            throw ParseError("Invalid integer");

        return result;
    }

    static void expectToken(const std::vector<uint8_t>& str,
                            std::vector<uint8_t>::const_iterator& iterator,
                            char token)
    {
        if (iterator == str.end() || *iterator != static_cast<uint8_t>(token))
            throw ParseError("Unexpected token");

        ++iterator;
    }

    BMFont::BMFont()
    {
    }

    BMFont::BMFont(const std::vector<uint8_t>& data)
    {
        auto iterator = data.cbegin();

        std::string keyword;
        std::string key;
        std::string value;

        for (;;)
        {
            if (iterator == data.end()) break;

            if (isNewline(*iterator))
            {
                // skip empty lines
                ++iterator;
            }
            else
            {
                skipWhitespaces(data, iterator);
                keyword = parseString(data, iterator);

                if (keyword == "page")
                {
                    for (;;)
                    {
                        if (iterator == data.end() || isNewline(*iterator)) break;

                        skipWhitespaces(data, iterator);
                        key = parseString(data, iterator);

                        expectToken(data, iterator, '=');
                        value = parseString(data, iterator);

                        if (key == "file")
                            fontTexture = engine->getCache().getTexture(value);
                    }
                }
                else if (keyword == "common")
                {
                    for (;;)
                    {
                        if (iterator == data.end() || isNewline(*iterator)) break;

                        skipWhitespaces(data, iterator);
                        key = parseString(data, iterator);

                        expectToken(data, iterator, '=');

                        if (key == "lineHeight")
                        {
                            value = parseInt(data, iterator);
                            lineHeight = static_cast<uint16_t>(std::stoi(value));
                        }
                        else if (key == "base")
                        {
                            value = parseInt(data, iterator);
                            base = static_cast<uint16_t>(std::stoi(value));
                        }
                        else if (key == "scaleW")
                        {
                            value = parseInt(data, iterator);
                            width = static_cast<uint16_t>(std::stoi(value));
                        }
                        else if (key == "scaleH")
                        {
                            value = parseInt(data, iterator);
                            height = static_cast<uint16_t>(std::stoi(value));
                        }
                        else if (key == "pages")
                        {
                            value = parseInt(data, iterator);
                            pages = static_cast<uint16_t>(std::stoi(value));
                        }
                        else if (key == "outline")
                        {
                            value = parseInt(data, iterator);
                            outline = static_cast<uint16_t>(std::stoi(value));
                        }
                        else
                            value = parseString(data, iterator);
                    }
                }
                else if (keyword == "char")
                {
                    uint32_t charId = 0;
                    CharDescriptor c;

                    for (;;)
                    {
                        if (iterator == data.end() || isNewline(*iterator)) break;

                        skipWhitespaces(data, iterator);
                        key = parseString(data, iterator);

                        expectToken(data, iterator, '=');

                        if (key == "id")
                        {
                            value = parseInt(data, iterator);
                            charId = static_cast<uint32_t>(std::stoul(value));
                        }
                        else if (key == "x")
                        {
                            value = parseInt(data, iterator);
                            c.x = static_cast<int16_t>(std::stoi(value));
                        }
                        else if (key == "y")
                        {
                            value = parseInt(data, iterator);
                            c.y = static_cast<int16_t>(std::stoi(value));
                        }
                        else if (key == "width")
                        {
                            value = parseInt(data, iterator);
                            c.width = static_cast<int16_t>(std::stoi(value));
                        }
                        else if (key == "height")
                        {
                            value = parseInt(data, iterator);
                            c.height = static_cast<int16_t>(std::stoi(value));
                        }
                        else if (key == "xoffset")
                        {
                            value = parseInt(data, iterator);
                            c.xOffset = static_cast<int16_t>(std::stoi(value));
                        }
                        else if (key == "yoffset")
                        {
                            value = parseInt(data, iterator);
                            c.yOffset = static_cast<int16_t>(std::stoi(value));
                        }
                        else if (key == "xadvance")
                        {
                            value = parseInt(data, iterator);
                            c.xAdvance = static_cast<int16_t>(std::stoi(value));
                        }
                        else if (key == "page")
                        {
                            value = parseInt(data, iterator);
                            c.page = static_cast<int16_t>(std::stoi(value));
                        }
                        else
                            value = parseString(data, iterator);
                    }

                    chars.insert(std::unordered_map<int32_t, CharDescriptor>::value_type(charId, c));
                }
                else if (keyword == "kernings")
                {
                    for (;;)
                    {
                        if (iterator == data.end() || isNewline(*iterator)) break;

                        skipWhitespaces(data, iterator);
                        key = parseString(data, iterator);

                        expectToken(data, iterator, '=');

                        if (key == "count")
                        {
                            value = parseInt(data, iterator);
                            kernCount = static_cast<uint16_t>(std::stoi(value));
                        }
                        else
                            value = parseString(data, iterator);
                    }
                }
                else if (keyword == "kerning")
                {
                    int16_t amount = 0;
                    uint32_t first = 0;
                    uint32_t second = 0;

                    for (;;)
                    {
                        if (iterator == data.end() || isNewline(*iterator)) break;

                        skipWhitespaces(data, iterator);
                        key = parseString(data, iterator);

                        expectToken(data, iterator, '=');

                        if (key == "first")
                        {
                            value = parseInt(data, iterator);
                            first = static_cast<uint32_t>(std::stoul(value));
                        }
                        else if (key == "second")
                        {
                            value = parseInt(data, iterator);
                            second = static_cast<uint32_t>(std::stoul(value));
                        }
                        else if (key == "amount")
                        {
                            value = parseInt(data, iterator);
                            amount = static_cast<int16_t>(std::stoi(value));
                        }
                        else
                            value = parseString(data, iterator);

                        kern[std::make_pair(first, second)] = amount;
                    }
                }
                else
                    skipLine(data, iterator);
            }
        }
    }

    void BMFont::getVertices(const std::string& text,
                             Color color,
                             float fontSize,
                             const Vector2& anchor,
                             std::vector<uint16_t>& indices,
                             std::vector<graphics::Vertex>& vertices,
                             std::shared_ptr<graphics::Texture>& texture)
    {
        Vector2 position;

        std::vector<uint32_t> utf32Text = utf8ToUtf32(text);

        indices.clear();
        vertices.clear();

        indices.reserve(utf32Text.size() * 6);
        vertices.reserve(utf32Text.size() * 4);

        Vector2 textCoords[4];

        size_t firstChar = 0;

        for (auto i = utf32Text.begin(); i != utf32Text.end(); ++i)
        {
            auto iter = chars.find(*i);

            if (iter != chars.end())
            {
                const CharDescriptor& f = iter->second;

                uint16_t startIndex = static_cast<uint16_t>(vertices.size());
                indices.push_back(startIndex + 0);
                indices.push_back(startIndex + 1);
                indices.push_back(startIndex + 2);

                indices.push_back(startIndex + 1);
                indices.push_back(startIndex + 3);
                indices.push_back(startIndex + 2);

                Vector2 leftTop(f.x / static_cast<float>(width),
                                f.y / static_cast<float>(height));

                Vector2 rightBottom((f.x + f.width) / static_cast<float>(width),
                                    (f.y + f.height) / static_cast<float>(height));

                textCoords[0] = Vector2(leftTop.x, rightBottom.y);
                textCoords[1] = Vector2(rightBottom.x, rightBottom.y);
                textCoords[2] = Vector2(leftTop.x, leftTop.y);
                textCoords[3] = Vector2(rightBottom.x, leftTop.y);

                vertices.push_back(graphics::Vertex(Vector3(position.x + f.xOffset, -position.y - f.yOffset - f.height, 0.0F),
                                                    color, textCoords[0], Vector3(0.0F, 0.0F, -1.0F)));
                vertices.push_back(graphics::Vertex(Vector3(position.x + f.xOffset + f.width, -position.y - f.yOffset - f.height, 0.0F),
                                                    color, textCoords[1], Vector3(0.0F, 0.0F, -1.0F)));
                vertices.push_back(graphics::Vertex(Vector3(position.x + f.xOffset, -position.y - f.yOffset, 0.0F),
                                                    color, textCoords[2], Vector3(0.0F, 0.0F, -1.0F)));
                vertices.push_back(graphics::Vertex(Vector3(position.x + f.xOffset + f.width, -position.y - f.yOffset, 0.0F),
                                                    color, textCoords[3], Vector3(0.0F, 0.0F, -1.0F)));

                if ((i + 1) != utf32Text.end())
                    position.x += static_cast<float>(getKerningPair(*i, *(i + 1)));

                position.x += f.xAdvance;
            }

            if (*i == static_cast<uint32_t>('\n') || // line feed
                (i + 1) == utf32Text.end()) // end of string
            {
                float lineWidth = position.x;
                position.x = 0.0F;
                position.y += lineHeight;

                for (size_t c = firstChar; c < vertices.size(); ++c)
                    vertices[c].position.x -= lineWidth * anchor.x;

                firstChar = vertices.size();
            }
        }

        float textHeight = position.y;

        for (size_t c = 0; c < vertices.size(); ++c)
        {
            vertices[c].position.y += textHeight * (1.0F - anchor.y);

            vertices[c].position.x *= fontSize;
            vertices[c].position.y *= fontSize;
        }

        texture = fontTexture;
    }

    int16_t BMFont::getKerningPair(uint32_t first, uint32_t second)
    {
        auto i = kern.find(std::make_pair(first, second));

        if (i != kern.end())
            return i->second;

        return 0;
    }

    float BMFont::getStringWidth(const std::string& text)
    {
        float total = 0.0F;

        std::vector<uint32_t> utf32Text = utf8ToUtf32(text);

        for (auto i = utf32Text.begin(); i != utf32Text.end(); ++i)
        {
            auto iter = chars.find(*i);

            if (iter != chars.end())
            {
                const CharDescriptor& f = iter->second;
                total += f.xAdvance;
            }
        }

        return total;
    }
}
