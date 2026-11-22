#include "Cosmetics.hpp"
#include "Config.hpp"
#include <fstream>
#include <cmath>
#include <algorithm>

namespace {
    constexpr float PI = 3.14159265358979323846f;
    const sf::Color outlineColor(35, 18, 10);

    sf::Vector2f rotatePoint(sf::Vector2f point, sf::Vector2f origin, float angleDeg) {
        float rad = angleDeg * PI / 180.0f;
        float s = std::sin(rad);
        float c = std::cos(rad);
        sf::Vector2f p = point - origin;
        return origin + sf::Vector2f(p.x * c - p.y * s, p.x * s + p.y * c);
    }
}

CosmeticsManager::CosmeticsManager()
    : m_equippedHat(HatId::None)
    , m_equippedOutfit(OutfitId::None)
    , m_apotheosisMode(false) {

    // Define all 13 Hats across score thresholds
    m_hats.push_back({ 0,  "No Headwear",         "Classic bare look", 0 });
    m_hats.push_back({ 1,  "Karate Headband",     "Vibrant crimson warrior band", 1 });
    m_hats.push_back({ 2,  "Aviator Goggles",     "Brass-rimmed pilot goggles", 15 });
    m_hats.push_back({ 3,  "Gentleman Top Hat",   "Silk top hat with ruby trim", 35 });
    m_hats.push_back({ 4,  "Cyber Neon Visor",    "Futuristic HUD laser visor", 65 });
    m_hats.push_back({ 5,  "Royal Golden Crown",  "Jeweled monarch crown", 100 });
    m_hats.push_back({ 6,  "Mystic Wizard Hat",   "Star-crested sorcerer cone", 150 });
    m_hats.push_back({ 7,  "Phoenix Fire Crown",  "Blazing fiery crown of embers", 220 });
    m_hats.push_back({ 8,  "Astronaut Bubble",    "Galactic pressurized helmet", 300 });
    m_hats.push_back({ 9,  "Celestial Halo",      "Radiant platinum starlight halo", 400 });
    m_hats.push_back({ 10, "Void Sovereign Horns","Curved obsidian horns with violet runes", 500 });
    m_hats.push_back({ 11, "Solar Emperor Diadem","Corona flare crown with sun spikes", 750 });
    m_hats.push_back({ 12, "Omnipotent Fool Crown","Divine apotheosis crown of the cosmos", 999 });

    // Define all 12 Outfits / Accessories / Auras
    m_outfits.push_back({ 0,  "No Outfit",         "Default aerodynamic body", 0 });
    m_outfits.push_back({ 1,  "Flight Scarf",      "Warm fluttering scarlet scarf", 15 });
    m_outfits.push_back({ 2,  "Dapper Bowtie",     "Formal ruby silk bowtie", 35 });
    m_outfits.push_back({ 3,  "Biker Vest",        "Studded leather vest", 65 });
    m_outfits.push_back({ 4,  "Velvet Royal Cape", "Flowing crimson monarch cape", 100 });
    m_outfits.push_back({ 5,  "Arcane Cowl",       "Starry mantle with rune circle", 150 });
    m_outfits.push_back({ 6,  "Phoenix Plumes",    "Fiery combustion plumes", 220 });
    m_outfits.push_back({ 7,  "Space Jetpack",     "Dual-thruster cyan rocket pack", 300 });
    m_outfits.push_back({ 8,  "Celestial Mantle",  "Angelic wings & starlight aura", 400 });
    m_outfits.push_back({ 9,  "Shadow Wings",      "Obsidian wings & crackling void", 500 });
    m_outfits.push_back({ 10, "Corona Wings",      "Solar plasma wings & corona orbit", 750 });
    m_outfits.push_back({ 11, "Supernova Aura",    "Divine prismatic apotheosis aura", 999 });

    load();
}

void CosmeticsManager::load() {
    try {
        std::ifstream file(Config::getCosmeticsFilePath());
        if (file.is_open()) {
            int hatInt = 0;
            int outfitInt = 0;
            int apoInt = 0;
            if (file >> hatInt >> outfitInt) {
                if (hatInt >= 0 && hatInt <= 12) m_equippedHat = static_cast<HatId>(hatInt);
                if (outfitInt >= 0 && outfitInt <= 11) m_equippedOutfit = static_cast<OutfitId>(outfitInt);
                if (file >> apoInt) m_apotheosisMode = (apoInt != 0);
            }
            file.close();
        }
    } catch (...) {
        // Silently ignore load errors
    }
}

void CosmeticsManager::save() {
    try {
        std::ofstream file(Config::getCosmeticsFilePath());
        if (file.is_open()) {
            file << static_cast<int>(m_equippedHat) << " "
                 << static_cast<int>(m_equippedOutfit) << " "
                 << (m_apotheosisMode ? 1 : 0) << std::endl;
            file.close();
        }
    } catch (...) {
        // Silently ignore save errors
    }
}

void CosmeticsManager::setEquippedHat(HatId hat) {
    m_equippedHat = hat;
    save();
}

void CosmeticsManager::setEquippedOutfit(OutfitId outfit) {
    m_equippedOutfit = outfit;
    save();
}

bool CosmeticsManager::isHatUnlocked(HatId hat, int bestScore) const {
    int idx = static_cast<int>(hat);
    if (idx >= 0 && idx < static_cast<int>(m_hats.size())) {
        return bestScore >= m_hats[idx].requiredScore;
    }
    return false;
}

bool CosmeticsManager::isOutfitUnlocked(OutfitId outfit, int bestScore) const {
    int idx = static_cast<int>(outfit);
    if (idx >= 0 && idx < static_cast<int>(m_outfits.size())) {
        return bestScore >= m_outfits[idx].requiredScore;
    }
    return false;
}

void CosmeticsManager::cycleNextHat(int bestScore) {
    int current = static_cast<int>(m_equippedHat);
    int total = static_cast<int>(m_hats.size());
    for (int i = 1; i <= total; ++i) {
        int next = (current + i) % total;
        if (isHatUnlocked(static_cast<HatId>(next), bestScore)) {
            setEquippedHat(static_cast<HatId>(next));
            return;
        }
    }
}

void CosmeticsManager::cyclePrevHat(int bestScore) {
    int current = static_cast<int>(m_equippedHat);
    int total = static_cast<int>(m_hats.size());
    for (int i = 1; i <= total; ++i) {
        int prev = (current - i + total) % total;
        if (isHatUnlocked(static_cast<HatId>(prev), bestScore)) {
            setEquippedHat(static_cast<HatId>(prev));
            return;
        }
    }
}

void CosmeticsManager::cycleNextOutfit(int bestScore) {
    int current = static_cast<int>(m_equippedOutfit);
    int total = static_cast<int>(m_outfits.size());
    for (int i = 1; i <= total; ++i) {
        int next = (current + i) % total;
        if (isOutfitUnlocked(static_cast<OutfitId>(next), bestScore)) {
            setEquippedOutfit(static_cast<OutfitId>(next));
            return;
        }
    }
}

void CosmeticsManager::cyclePrevOutfit(int bestScore) {
    int current = static_cast<int>(m_equippedOutfit);
    int total = static_cast<int>(m_outfits.size());
    for (int i = 1; i <= total; ++i) {
        int prev = (current - i + total) % total;
        if (isOutfitUnlocked(static_cast<OutfitId>(prev), bestScore)) {
            setEquippedOutfit(static_cast<OutfitId>(prev));
            return;
        }
    }
}

void CosmeticsManager::drawHat(sf::RenderWindow& window, sf::Vector2f headPos, float angle, float scale, HatId hat, float time) const {
    if (hat == HatId::None) return;

    sf::Transform t;
    t.translate(headPos);
    t.rotate(angle);
    t.scale(scale, scale);

    switch (hat) {
        case HatId::Headband: {
            // Crimson martial headband
            sf::RectangleShape band(sf::Vector2f(26.0f, 6.0f));
            band.setOrigin(13.0f, 3.0f);
            band.setPosition(2.0f, -8.0f);
            band.setFillColor(sf::Color(230, 30, 45));
            band.setOutlineThickness(1.5f);
            band.setOutlineColor(outlineColor);
            window.draw(band, t);

            // Fluttering tail ties
            float tailWiggle = std::sin(time * 12.0f) * 4.0f;
            sf::ConvexShape tail;
            tail.setPointCount(4);
            tail.setPoint(0, sf::Vector2f(-12.0f, -8.0f));
            tail.setPoint(1, sf::Vector2f(-22.0f, -5.0f + tailWiggle));
            tail.setPoint(2, sf::Vector2f(-24.0f, -1.0f + tailWiggle));
            tail.setPoint(3, sf::Vector2f(-12.0f, -6.0f));
            tail.setFillColor(sf::Color(210, 20, 35));
            tail.setOutlineThickness(1.2f);
            tail.setOutlineColor(outlineColor);
            window.draw(tail, t);
            break;
        }
        case HatId::AviatorGoggles: {
            // Brass goggles
            sf::RectangleShape strap(sf::Vector2f(28.0f, 4.0f));
            strap.setOrigin(14.0f, 2.0f);
            strap.setPosition(1.0f, -7.0f);
            strap.setFillColor(sf::Color(90, 50, 25));
            window.draw(strap, t);

            sf::CircleShape lens(6.5f);
            lens.setOrigin(6.5f, 6.5f);
            lens.setPosition(4.0f, -7.0f);
            lens.setFillColor(sf::Color(40, 220, 240, 220));
            lens.setOutlineThickness(2.0f);
            lens.setOutlineColor(sf::Color(215, 160, 45));
            window.draw(lens, t);

            // Glass glint
            sf::CircleShape glint(2.2f);
            glint.setOrigin(1.1f, 1.1f);
            glint.setPosition(2.5f, -9.0f);
            glint.setFillColor(sf::Color(255, 255, 255, 240));
            window.draw(glint, t);
            break;
        }
        case HatId::TopHat: {
            // Brim
            sf::RectangleShape brim(sf::Vector2f(32.0f, 4.0f));
            brim.setOrigin(16.0f, 2.0f);
            brim.setPosition(0.0f, -12.0f);
            brim.setFillColor(sf::Color(25, 20, 30));
            brim.setOutlineThickness(1.5f);
            brim.setOutlineColor(outlineColor);
            window.draw(brim, t);

            // Crown
            sf::RectangleShape crown(sf::Vector2f(20.0f, 18.0f));
            crown.setOrigin(10.0f, 18.0f);
            crown.setPosition(0.0f, -12.0f);
            crown.setFillColor(sf::Color(35, 30, 40));
            crown.setOutlineThickness(1.5f);
            crown.setOutlineColor(outlineColor);
            window.draw(crown, t);

            // Ribbon
            sf::RectangleShape ribbon(sf::Vector2f(20.0f, 4.0f));
            ribbon.setOrigin(10.0f, 4.0f);
            ribbon.setPosition(0.0f, -12.0f);
            ribbon.setFillColor(sf::Color(210, 25, 45));
            window.draw(ribbon, t);
            break;
        }
        case HatId::CyberVisor: {
            // Neon cyan/magenta visor
            sf::ConvexShape visor;
            visor.setPointCount(4);
            visor.setPoint(0, sf::Vector2f(-4.0f, -10.0f));
            visor.setPoint(1, sf::Vector2f(14.0f, -8.0f));
            visor.setPoint(2, sf::Vector2f(12.0f, -1.0f));
            visor.setPoint(3, sf::Vector2f(-2.0f, -3.0f));
            visor.setFillColor(sf::Color(0, 245, 255, 230));
            visor.setOutlineThickness(2.0f);
            visor.setOutlineColor(sf::Color(255, 20, 147));
            window.draw(visor, t);

            // Scanline streak
            float pulse = (std::sin(time * 10.0f) + 1.0f) * 0.5f;
            sf::RectangleShape scanline(sf::Vector2f(14.0f, 2.0f));
            scanline.setOrigin(7.0f, 1.0f);
            scanline.setPosition(5.0f, -5.0f);
            scanline.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(140 + pulse * 115)));
            window.draw(scanline, t);
            break;
        }
        case HatId::RoyalCrown: {
            // Golden crown with jeweled points
            sf::ConvexShape crown;
            crown.setPointCount(7);
            crown.setPoint(0, sf::Vector2f(-12.0f, -10.0f));
            crown.setPoint(1, sf::Vector2f(-12.0f, -22.0f));
            crown.setPoint(2, sf::Vector2f(-5.0f,  -15.0f));
            crown.setPoint(3, sf::Vector2f(0.0f,   -25.0f));
            crown.setPoint(4, sf::Vector2f(5.0f,   -15.0f));
            crown.setPoint(5, sf::Vector2f(12.0f,  -22.0f));
            crown.setPoint(6, sf::Vector2f(12.0f,  -10.0f));
            crown.setFillColor(sf::Color(255, 215, 0));
            crown.setOutlineThickness(1.8f);
            crown.setOutlineColor(outlineColor);
            window.draw(crown, t);

            // Ruby jewel
            sf::CircleShape ruby(2.5f);
            ruby.setOrigin(1.25f, 1.25f);
            ruby.setPosition(0.0f, -17.0f);
            ruby.setFillColor(sf::Color(230, 25, 45));
            window.draw(ruby, t);
            break;
        }
        case HatId::WizardHat: {
            // Indigo star cone
            sf::ConvexShape cone;
            cone.setPointCount(4);
            cone.setPoint(0, sf::Vector2f(-14.0f, -11.0f));
            cone.setPoint(1, sf::Vector2f(-5.0f,  -34.0f));
            cone.setPoint(2, sf::Vector2f(2.0f,   -32.0f));
            cone.setPoint(3, sf::Vector2f(12.0f,  -11.0f));
            cone.setFillColor(sf::Color(55, 35, 110));
            cone.setOutlineThickness(1.8f);
            cone.setOutlineColor(outlineColor);
            window.draw(cone, t);

            // Wide brim
            sf::RectangleShape brim(sf::Vector2f(34.0f, 4.0f));
            brim.setOrigin(17.0f, 2.0f);
            brim.setPosition(-1.0f, -11.0f);
            brim.setFillColor(sf::Color(75, 45, 145));
            brim.setOutlineThickness(1.5f);
            brim.setOutlineColor(outlineColor);
            window.draw(brim, t);

            // Star spark on tip
            sf::CircleShape star(3.0f);
            star.setOrigin(1.5f, 1.5f);
            star.setPosition(-2.0f, -33.0f);
            star.setFillColor(sf::Color(255, 235, 70));
            window.draw(star, t);
            break;
        }
        case HatId::PhoenixCrown: {
            // Animated fiery flame tips
            for (int i = -2; i <= 2; ++i) {
                float wave = std::sin(time * 14.0f + i * 1.3f) * 3.5f;
                float height = 15.0f + (2 - std::abs(i)) * 6.0f;
                sf::ConvexShape flame;
                flame.setPointCount(3);
                flame.setPoint(0, sf::Vector2f(i * 5.0f - 3.0f, -10.0f));
                flame.setPoint(1, sf::Vector2f(i * 5.0f + wave, -10.0f - height));
                flame.setPoint(2, sf::Vector2f(i * 5.0f + 3.0f, -10.0f));
                flame.setFillColor((std::abs(i) <= 1) ? sf::Color(255, 230, 80, 240) : sf::Color(255, 80, 20, 220));
                window.draw(flame, t);
            }
            break;
        }
        case HatId::Astronaut: {
            // Iridescent glass dome
            sf::CircleShape dome(18.0f);
            dome.setOrigin(18.0f, 18.0f);
            dome.setPosition(0.0f, -2.0f);
            dome.setFillColor(sf::Color(140, 210, 255, 110));
            dome.setOutlineThickness(2.5f);
            dome.setOutlineColor(sf::Color(200, 220, 240, 230));
            window.draw(dome, t);

            // Glass reflection highlight
            sf::CircleShape glint(5.0f);
            glint.setOrigin(2.5f, 2.5f);
            glint.setPosition(-7.0f, -9.0f);
            glint.setFillColor(sf::Color(255, 255, 255, 180));
            window.draw(glint, t);
            break;
        }
        case HatId::CelestialHalo: {
            // Floating platinum-white halo
            float floatY = -26.0f + std::sin(time * 5.0f) * 2.5f;
            sf::CircleShape halo(16.0f);
            halo.setOrigin(16.0f, 7.0f);
            halo.setScale(1.0f, 0.42f);
            halo.setPosition(0.0f, floatY);
            halo.setFillColor(sf::Color::Transparent);
            halo.setOutlineThickness(3.5f);
            halo.setOutlineColor(sf::Color(245, 245, 255, 240));
            window.draw(halo, t);

            // Golden star glints
            sf::CircleShape spark(2.5f);
            spark.setFillColor(sf::Color(255, 235, 140));
            spark.setPosition(-12.0f, floatY - 2.0f);
            window.draw(spark, t);
            spark.setPosition(8.0f, floatY - 2.0f);
            window.draw(spark, t);
            break;
        }
        case HatId::VoidHorns: {
            // Twin obsidian curved demon horns
            for (int side = -1; side <= 1; side += 2) {
                sf::ConvexShape horn;
                horn.setPointCount(4);
                horn.setPoint(0, sf::Vector2f(side * 6.0f, -10.0f));
                horn.setPoint(1, sf::Vector2f(side * 14.0f, -24.0f));
                horn.setPoint(2, sf::Vector2f(side * 10.0f, -27.0f));
                horn.setPoint(3, sf::Vector2f(side * 2.0f, -13.0f));
                horn.setFillColor(sf::Color(20, 15, 30));
                horn.setOutlineThickness(1.8f);
                horn.setOutlineColor(sf::Color(170, 30, 240));
                window.draw(horn, t);
            }
            break;
        }
        case HatId::SolarDiadem: {
            // Golden solar corona diadem
            sf::CircleShape disc(9.0f);
            disc.setOrigin(9.0f, 9.0f);
            disc.setPosition(0.0f, -16.0f);
            disc.setFillColor(sf::Color(255, 200, 30));
            disc.setOutlineThickness(2.0f);
            disc.setOutlineColor(sf::Color(255, 90, 0));
            window.draw(disc, t);

            for (int r = 0; r < 8; ++r) {
                float rot = time * 30.0f + r * (360.0f / 8.0f);
                sf::RectangleShape ray(sf::Vector2f(5.0f, 2.0f));
                ray.setOrigin(0.0f, 1.0f);
                ray.setPosition(0.0f, -16.0f);
                ray.setRotation(rot);
                ray.setFillColor(sf::Color(255, 230, 80, 220));
                window.draw(ray, t);
            }
            break;
        }
        case HatId::OmnipotentCrown: {
            // Omnipotent Fool's Divine Apex Crown
            float pulse = (std::sin(time * 6.0f) + 1.0f) * 0.5f;

            // Triple-tiered golden crystalline crown
            sf::ConvexShape crown;
            crown.setPointCount(7);
            crown.setPoint(0, sf::Vector2f(-15.0f, -11.0f));
            crown.setPoint(1, sf::Vector2f(-16.0f, -28.0f - pulse * 3.0f));
            crown.setPoint(2, sf::Vector2f(-6.0f,  -19.0f));
            crown.setPoint(3, sf::Vector2f(0.0f,   -35.0f - pulse * 4.0f));
            crown.setPoint(4, sf::Vector2f(6.0f,   -19.0f));
            crown.setPoint(5, sf::Vector2f(16.0f,  -28.0f - pulse * 3.0f));
            crown.setPoint(6, sf::Vector2f(15.0f,  -11.0f));
            crown.setFillColor(sf::Color(255, 245, 180));
            crown.setOutlineThickness(2.2f);
            crown.setOutlineColor(sf::Color(215, 160, 20));
            window.draw(crown, t);

            // Eye of Mystery diamond gem
            sf::CircleShape eye(4.0f, 4); // diamond
            eye.setOrigin(4.0f, 4.0f);
            eye.setPosition(0.0f, -21.0f);
            eye.setFillColor(sf::Color(140, 235, 255));
            eye.setOutlineThickness(1.5f);
            eye.setOutlineColor(sf::Color(255, 215, 0));
            window.draw(eye, t);

            // Starlight crown halo
            sf::CircleShape starHalo(22.0f);
            starHalo.setOrigin(22.0f, 10.0f);
            starHalo.setScale(1.0f, 0.45f);
            starHalo.setPosition(0.0f, -32.0f);
            starHalo.setFillColor(sf::Color::Transparent);
            starHalo.setOutlineThickness(2.0f);
            starHalo.setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(160 + pulse * 90)));
            window.draw(starHalo, t);
            break;
        }
    }
}

void CosmeticsManager::drawOutfit(sf::RenderWindow& window, sf::Vector2f bodyPos, float angle, float scale, OutfitId outfit, float time) const {
    if (outfit == OutfitId::None) return;

    sf::Transform t;
    t.translate(bodyPos);
    t.rotate(angle);
    t.scale(scale, scale);

    switch (outfit) {
        case OutfitId::AviatorScarf: {
            // Scarlet wrapped scarf around neck
            sf::RectangleShape wrap(sf::Vector2f(16.0f, 7.0f));
            wrap.setOrigin(8.0f, 3.5f);
            wrap.setPosition(1.0f, 4.0f);
            wrap.setFillColor(sf::Color(215, 25, 45));
            wrap.setOutlineThickness(1.5f);
            wrap.setOutlineColor(outlineColor);
            window.draw(wrap, t);

            // Animated waving tail
            float tailWiggle = std::sin(time * 10.0f) * 6.0f;
            sf::ConvexShape tail;
            tail.setPointCount(4);
            tail.setPoint(0, sf::Vector2f(-6.0f, 4.0f));
            tail.setPoint(1, sf::Vector2f(-22.0f, 8.0f + tailWiggle));
            tail.setPoint(2, sf::Vector2f(-24.0f, 15.0f + tailWiggle));
            tail.setPoint(3, sf::Vector2f(-6.0f, 8.0f));
            tail.setFillColor(sf::Color(190, 20, 35));
            tail.setOutlineThickness(1.2f);
            tail.setOutlineColor(outlineColor);
            window.draw(tail, t);
            break;
        }
        case OutfitId::DapperBowtie: {
            // Formal ruby bowtie
            sf::ConvexShape leftWing;
            leftWing.setPointCount(3);
            leftWing.setPoint(0, sf::Vector2f(5.0f, 7.0f));
            leftWing.setPoint(1, sf::Vector2f(-3.0f, 3.0f));
            leftWing.setPoint(2, sf::Vector2f(-3.0f, 11.0f));
            leftWing.setFillColor(sf::Color(200, 20, 40));
            leftWing.setOutlineThickness(1.2f);
            leftWing.setOutlineColor(outlineColor);
            window.draw(leftWing, t);

            sf::ConvexShape rightWing;
            rightWing.setPointCount(3);
            rightWing.setPoint(0, sf::Vector2f(5.0f, 7.0f));
            rightWing.setPoint(1, sf::Vector2f(13.0f, 3.0f));
            rightWing.setPoint(2, sf::Vector2f(13.0f, 11.0f));
            rightWing.setFillColor(sf::Color(200, 20, 40));
            rightWing.setOutlineThickness(1.2f);
            rightWing.setOutlineColor(outlineColor);
            window.draw(rightWing, t);

            sf::CircleShape knot(3.0f);
            knot.setOrigin(1.5f, 1.5f);
            knot.setPosition(5.0f, 7.0f);
            knot.setFillColor(sf::Color(255, 215, 0));
            window.draw(knot, t);
            break;
        }
        case OutfitId::BikerVest: {
            // Studded leather vest
            sf::RectangleShape vest(sf::Vector2f(20.0f, 18.0f));
            vest.setOrigin(10.0f, 9.0f);
            vest.setPosition(-1.0f, 2.0f);
            vest.setFillColor(sf::Color(35, 30, 40));
            vest.setOutlineThickness(1.5f);
            vest.setOutlineColor(outlineColor);
            window.draw(vest, t);

            // Silver studs
            sf::CircleShape stud(1.8f);
            stud.setFillColor(sf::Color(210, 215, 225));
            stud.setPosition(-8.0f, -4.0f); window.draw(stud, t);
            stud.setPosition(-8.0f, 4.0f);  window.draw(stud, t);
            stud.setPosition(4.0f, -4.0f);  window.draw(stud, t);
            stud.setPosition(4.0f, 4.0f);   window.draw(stud, t);
            break;
        }
        case OutfitId::RoyalCape: {
            // Flowing royal purple/crimson cape
            float capeWave = std::sin(time * 8.0f) * 5.0f;
            sf::ConvexShape cape;
            cape.setPointCount(4);
            cape.setPoint(0, sf::Vector2f(-4.0f, 0.0f));
            cape.setPoint(1, sf::Vector2f(-26.0f, 10.0f + capeWave));
            cape.setPoint(2, sf::Vector2f(-22.0f, 24.0f + capeWave));
            cape.setPoint(3, sf::Vector2f(-2.0f, 12.0f));
            cape.setFillColor(sf::Color(140, 20, 60));
            cape.setOutlineThickness(1.8f);
            cape.setOutlineColor(sf::Color(255, 215, 0));
            window.draw(cape, t);
            break;
        }
        case OutfitId::ArcaneCowl: {
            // Starry purple mantle
            sf::ConvexShape cowl;
            cowl.setPointCount(5);
            cowl.setPoint(0, sf::Vector2f(8.0f, 2.0f));
            cowl.setPoint(1, sf::Vector2f(-12.0f, 0.0f));
            cowl.setPoint(2, sf::Vector2f(-18.0f, 16.0f));
            cowl.setPoint(3, sf::Vector2f(-2.0f, 18.0f));
            cowl.setPoint(4, sf::Vector2f(10.0f, 10.0f));
            cowl.setFillColor(sf::Color(45, 25, 95));
            cowl.setOutlineThickness(1.5f);
            cowl.setOutlineColor(sf::Color(240, 200, 50));
            window.draw(cowl, t);
            break;
        }
        case OutfitId::PhoenixPlumes: {
            // Combustion plumes
            for (int i = 0; i < 3; ++i) {
                float wave = std::sin(time * 12.0f + i * 1.5f) * 4.0f;
                sf::ConvexShape plume;
                plume.setPointCount(3);
                plume.setPoint(0, sf::Vector2f(-10.0f, 2.0f + i * 4.0f));
                plume.setPoint(1, sf::Vector2f(-28.0f - i * 4.0f, 4.0f + i * 3.0f + wave));
                plume.setPoint(2, sf::Vector2f(-12.0f, 8.0f + i * 4.0f));
                plume.setFillColor((i == 0) ? sf::Color(255, 120, 20, 230) : sf::Color(255, 210, 40, 210));
                window.draw(plume, t);
            }
            break;
        }
        case OutfitId::SpaceJetpack: {
            // Dual titanium rockets
            sf::RectangleShape thruster(sf::Vector2f(10.0f, 16.0f));
            thruster.setOrigin(5.0f, 8.0f);
            thruster.setPosition(-12.0f, 4.0f);
            thruster.setFillColor(sf::Color(170, 180, 195));
            thruster.setOutlineThickness(1.5f);
            thruster.setOutlineColor(outlineColor);
            window.draw(thruster, t);

            // Pulsing cyan rocket exhaust
            float exhaustPulse = (std::sin(time * 20.0f) + 1.0f) * 4.0f;
            sf::ConvexShape fire;
            fire.setPointCount(3);
            fire.setPoint(0, sf::Vector2f(-16.0f, 12.0f));
            fire.setPoint(1, sf::Vector2f(-12.0f, 22.0f + exhaustPulse));
            fire.setPoint(2, sf::Vector2f(-8.0f,  12.0f));
            fire.setFillColor(sf::Color(40, 230, 255, 230));
            window.draw(fire, t);
            break;
        }
        case OutfitId::CelestialMantle: {
            // Radiant feathered wings
            float flapOffset = std::sin(time * 10.0f) * 6.0f;
            sf::ConvexShape wing;
            wing.setPointCount(5);
            wing.setPoint(0, sf::Vector2f(-4.0f, -4.0f));
            wing.setPoint(1, sf::Vector2f(-24.0f, -14.0f + flapOffset));
            wing.setPoint(2, sf::Vector2f(-32.0f, 2.0f + flapOffset));
            wing.setPoint(3, sf::Vector2f(-22.0f, 12.0f));
            wing.setPoint(4, sf::Vector2f(-6.0f, 6.0f));
            wing.setFillColor(sf::Color(245, 245, 255, 220));
            wing.setOutlineThickness(1.8f);
            wing.setOutlineColor(sf::Color(255, 215, 0));
            window.draw(wing, t);
            break;
        }
        case OutfitId::ShadowWings: {
            // Dragon-like dark matter wings
            float flapOffset = std::sin(time * 9.0f) * 7.0f;
            sf::ConvexShape wing;
            wing.setPointCount(5);
            wing.setPoint(0, sf::Vector2f(-4.0f, -2.0f));
            wing.setPoint(1, sf::Vector2f(-26.0f, -18.0f + flapOffset));
            wing.setPoint(2, sf::Vector2f(-34.0f, -2.0f + flapOffset));
            wing.setPoint(3, sf::Vector2f(-20.0f, 12.0f));
            wing.setPoint(4, sf::Vector2f(-6.0f, 6.0f));
            wing.setFillColor(sf::Color(25, 15, 35, 230));
            wing.setOutlineThickness(2.0f);
            wing.setOutlineColor(sf::Color(165, 30, 235));
            window.draw(wing, t);
            break;
        }
        case OutfitId::CoronaWings: {
            // Blazing solar wings
            float flap = std::sin(time * 11.0f) * 6.0f;
            sf::ConvexShape wing;
            wing.setPointCount(5);
            wing.setPoint(0, sf::Vector2f(-4.0f, -2.0f));
            wing.setPoint(1, sf::Vector2f(-26.0f, -16.0f + flap));
            wing.setPoint(2, sf::Vector2f(-32.0f, 4.0f + flap));
            wing.setPoint(3, sf::Vector2f(-18.0f, 12.0f));
            wing.setPoint(4, sf::Vector2f(-6.0f, 6.0f));
            wing.setFillColor(sf::Color(255, 140, 20, 220));
            wing.setOutlineThickness(2.0f);
            wing.setOutlineColor(sf::Color(255, 230, 60));
            window.draw(wing, t);
            break;
        }
        case OutfitId::SupernovaAura: {
            // Divine Apotheosis wings + starlight trail
            float flap = std::sin(time * 8.0f) * 7.0f;
            sf::ConvexShape wing;
            wing.setPointCount(6);
            wing.setPoint(0, sf::Vector2f(-4.0f, -2.0f));
            wing.setPoint(1, sf::Vector2f(-24.0f, -20.0f + flap));
            wing.setPoint(2, sf::Vector2f(-36.0f, -6.0f + flap));
            wing.setPoint(3, sf::Vector2f(-30.0f, 10.0f + flap));
            wing.setPoint(4, sf::Vector2f(-16.0f, 14.0f));
            wing.setPoint(5, sf::Vector2f(-4.0f, 6.0f));
            wing.setFillColor(sf::Color(255, 255, 255, 240));
            wing.setOutlineThickness(2.2f);
            wing.setOutlineColor(sf::Color(255, 215, 80));
            window.draw(wing, t);
            break;
        }
    }
}

void CosmeticsManager::drawAuras(sf::RenderWindow& window, sf::Vector2f bodyPos, OutfitId outfit, HatId hat, float time) const {
    // 1. Arcane Rune Circle (Outfit 5)
    if (outfit == OutfitId::ArcaneCowl) {
        sf::CircleShape runeRing(26.0f);
        runeRing.setOrigin(26.0f, 26.0f);
        runeRing.setPosition(bodyPos);
        runeRing.setFillColor(sf::Color::Transparent);
        runeRing.setOutlineThickness(2.0f);
        runeRing.setOutlineColor(sf::Color(170, 70, 255, 140));
        window.draw(runeRing);

        // Orbiting rune nodes
        for (int i = 0; i < 4; ++i) {
            float rot = time * 3.0f + i * (PI * 0.5f);
            sf::Vector2f nodePos = bodyPos + sf::Vector2f(std::cos(rot) * 26.0f, std::sin(rot) * 26.0f);
            sf::CircleShape node(2.5f);
            node.setOrigin(1.25f, 1.25f);
            node.setPosition(nodePos);
            node.setFillColor(sf::Color(255, 230, 80, 220));
            window.draw(node);
        }
    }

    // 2. Phoenix Fire Aura (Outfit 6 or Hat 7)
    if (outfit == OutfitId::PhoenixPlumes || hat == HatId::PhoenixCrown) {
        float pulse = (std::sin(time * 8.0f) + 1.0f) * 0.5f;
        sf::CircleShape fireRing(28.0f + pulse * 4.0f);
        fireRing.setOrigin(fireRing.getRadius(), fireRing.getRadius());
        fireRing.setPosition(bodyPos);
        fireRing.setFillColor(sf::Color(255, 100, 20, static_cast<sf::Uint8>(40 + pulse * 35)));
        fireRing.setOutlineThickness(2.0f);
        fireRing.setOutlineColor(sf::Color(255, 200, 40, static_cast<sf::Uint8>(120 + pulse * 60)));
        window.draw(fireRing);
    }

    // 3. Celestial Starlight Aura (Outfit 8 or Hat 9)
    if (outfit == OutfitId::CelestialMantle || hat == HatId::CelestialHalo) {
        float pulse = (std::sin(time * 5.0f) + 1.0f) * 0.5f;
        sf::CircleShape starlight(30.0f + pulse * 3.0f);
        starlight.setOrigin(starlight.getRadius(), starlight.getRadius());
        starlight.setPosition(bodyPos);
        starlight.setFillColor(sf::Color(220, 240, 255, static_cast<sf::Uint8>(35 + pulse * 30)));
        starlight.setOutlineThickness(1.8f);
        starlight.setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(140 + pulse * 70)));
        window.draw(starlight);
    }

    // 4. Void Shadow Aura (Outfit 9 or Hat 10)
    if (outfit == OutfitId::ShadowWings || hat == HatId::VoidHorns) {
        float pulse = (std::sin(time * 7.0f) + 1.0f) * 0.5f;
        sf::CircleShape voidAura(30.0f + pulse * 5.0f);
        voidAura.setOrigin(voidAura.getRadius(), voidAura.getRadius());
        voidAura.setPosition(bodyPos);
        voidAura.setFillColor(sf::Color(50, 15, 80, static_cast<sf::Uint8>(55 + pulse * 40)));
        voidAura.setOutlineThickness(2.5f);
        voidAura.setOutlineColor(sf::Color(190, 40, 255, static_cast<sf::Uint8>(130 + pulse * 80)));
        window.draw(voidAura);
    }

    // 5. Solar Corona Aura (Outfit 10 or Hat 11)
    if (outfit == OutfitId::CoronaWings || hat == HatId::SolarDiadem) {
        float pulse = (std::sin(time * 9.0f) + 1.0f) * 0.5f;
        sf::CircleShape corona(32.0f + pulse * 4.0f);
        corona.setOrigin(corona.getRadius(), corona.getRadius());
        corona.setPosition(bodyPos);
        corona.setFillColor(sf::Color(255, 140, 0, static_cast<sf::Uint8>(45 + pulse * 35)));
        corona.setOutlineThickness(2.2f);
        corona.setOutlineColor(sf::Color(255, 235, 70, static_cast<sf::Uint8>(150 + pulse * 75)));
        window.draw(corona);
    }

    // 6. Omnipotent Supernova Apotheosis Aura (Outfit 11 or Hat 12)
    if (outfit == OutfitId::SupernovaAura || hat == HatId::OmnipotentCrown) {
        float pulse = (std::sin(time * 6.0f) + 1.0f) * 0.5f;

        // Concentric divine rings
        sf::CircleShape innerRing(28.0f + pulse * 4.0f);
        innerRing.setOrigin(innerRing.getRadius(), innerRing.getRadius());
        innerRing.setPosition(bodyPos);
        innerRing.setFillColor(sf::Color(255, 250, 210, static_cast<sf::Uint8>(50 + pulse * 45)));
        innerRing.setOutlineThickness(2.5f);
        innerRing.setOutlineColor(sf::Color(255, 215, 50, static_cast<sf::Uint8>(170 + pulse * 70)));
        window.draw(innerRing);

        sf::CircleShape outerRing(38.0f + pulse * 6.0f);
        outerRing.setOrigin(outerRing.getRadius(), outerRing.getRadius());
        outerRing.setPosition(bodyPos);
        outerRing.setFillColor(sf::Color::Transparent);
        outerRing.setOutlineThickness(1.8f);
        outerRing.setOutlineColor(sf::Color(160, 235, 255, static_cast<sf::Uint8>(140 + pulse * 80)));
        window.draw(outerRing);

        // Orbiting supernova stars
        for (int i = 0; i < 6; ++i) {
            float rot = time * 4.0f + i * (PI / 3.0f);
            sf::Vector2f starPos = bodyPos + sf::Vector2f(std::cos(rot) * (34.0f + pulse * 5.0f),
                                                          std::sin(rot) * (34.0f + pulse * 5.0f));
            sf::CircleShape star(2.8f, 4); // diamond
            star.setOrigin(1.4f, 1.4f);
            star.setPosition(starPos);
            star.setFillColor(sf::Color(255, 255, 255, 230));
            window.draw(star);
        }
    }
}
