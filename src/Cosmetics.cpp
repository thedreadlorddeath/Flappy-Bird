#include "Cosmetics.hpp"
#include <fstream>
#include <cmath>
#include <algorithm>

namespace {
    const char* COSMETICS_FILE = "cosmetics.txt";
    const sf::Color outlineColor(35, 18, 10);
}

CosmeticsManager::CosmeticsManager()
    : m_equippedHat(HatId::None)
    , m_equippedOutfit(OutfitId::None) {

    // Define all 9 Hats across score thresholds
    m_hats.push_back({ 0, "No Headwear", "Classic bare look", 0 });
    m_hats.push_back({ 1, "Karate Headband", "Red warrior headband", 1 });
    m_hats.push_back({ 2, "Aviator Goggles", "Brass-rimmed pilot goggles", 5 });
    m_hats.push_back({ 3, "Gentleman Top Hat", "Silk top hat with crimson band", 10 });
    m_hats.push_back({ 4, "Cyber Neon Visor", "Futuristic HUD laser visor", 15 });
    m_hats.push_back({ 5, "Royal Golden Crown", "Jeweled monarch crown", 20 });
    m_hats.push_back({ 6, "Mystic Wizard Hat", "Star-crested sorcerer cone", 25 });
    m_hats.push_back({ 7, "Phoenix Fire Halo", "Blazing fiery crown of embers", 30 });
    m_hats.push_back({ 8, "Astronaut Bubble", "Galactic pressurized helmet", 35 });
    m_hats.push_back({ 9, "Celestial Platinum Halo", "Mythic angelic starlight halo", 40 });

    // Define all 8 Outfits / Jackets / Accessories
    m_outfits.push_back({ 0, "No Outfit", "Default aerodynamic body", 0 });
    m_outfits.push_back({ 1, "Flight Scarf", "Warm fluttering aviator scarf", 5 });
    m_outfits.push_back({ 2, "Dapper Bowtie", "Formal ruby silk bowtie", 10 });
    m_outfits.push_back({ 3, "Biker Vest", "Leather vest with silver studs", 15 });
    m_outfits.push_back({ 4, "Velvet Royal Cape", "Flowing crimson monarch cape", 20 });
    m_outfits.push_back({ 5, "Arcane Cowl", "Starry purple mystic mantle", 25 });
    m_outfits.push_back({ 6, "Phoenix Flame Plumes", "Fiery combustion trail plumes", 30 });
    m_outfits.push_back({ 7, "Space Jetpack", "Dual-thruster booster rockets", 35 });
    m_outfits.push_back({ 8, "Celestial Mantle", "Angelic glowing starlight wings", 40 });

    load();
}

void CosmeticsManager::load() {
    std::ifstream file(COSMETICS_FILE);
    if (file.is_open()) {
        int hatInt = 0;
        int outfitInt = 0;
        if (file >> hatInt >> outfitInt) {
            if (hatInt >= 0 && hatInt <= 9) m_equippedHat = static_cast<HatId>(hatInt);
            if (outfitInt >= 0 && outfitInt <= 8) m_equippedOutfit = static_cast<OutfitId>(outfitInt);
        }
        file.close();
    }
}

void CosmeticsManager::save() {
    std::ofstream file(COSMETICS_FILE);
    if (file.is_open()) {
        file << static_cast<int>(m_equippedHat) << " " << static_cast<int>(m_equippedOutfit) << std::endl;
        file.close();
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

std::string CosmeticsManager::getHatName(HatId hat) const {
    int idx = static_cast<int>(hat);
    if (idx >= 0 && idx < static_cast<int>(m_hats.size())) {
        return m_hats[idx].name;
    }
    return "None";
}

std::string CosmeticsManager::getOutfitName(OutfitId outfit) const {
    int idx = static_cast<int>(outfit);
    if (idx >= 0 && idx < static_cast<int>(m_outfits.size())) {
        return m_outfits[idx].name;
    }
    return "None";
}

void CosmeticsManager::drawHat(sf::RenderWindow& window, const sf::Transform& birdTransform, HatId hat, float time) {
    if (hat == HatId::None) return;

    sf::RenderStates states;
    states.transform = birdTransform;

    switch (hat) {
    case HatId::Headband: {
        // Red fighter headband wrapped across forehead
        sf::RectangleShape band(sf::Vector2f(28.0f, 5.0f));
        band.setOrigin(14.0f, 2.5f);
        band.setPosition(4.0f, -12.0f);
        band.setFillColor(sf::Color(235, 45, 45));
        band.setOutlineColor(outlineColor);
        band.setOutlineThickness(1.2f);
        window.draw(band, states);

        // Trailing headband ribbons fluttering behind
        float flutter = std::sin(time * 18.0f) * 3.0f;
        sf::ConvexShape ribbon1;
        ribbon1.setPointCount(3);
        ribbon1.setPoint(0, sf::Vector2f(-10.0f, -12.0f));
        ribbon1.setPoint(1, sf::Vector2f(-24.0f, -16.0f + flutter));
        ribbon1.setPoint(2, sf::Vector2f(-18.0f, -8.0f + flutter));
        ribbon1.setFillColor(sf::Color(215, 35, 35));
        window.draw(ribbon1, states);
        break;
    }
    case HatId::AviatorGoggles: {
        // Brass framed dual goggles on head
        sf::RectangleShape strap(sf::Vector2f(30.0f, 4.0f));
        strap.setOrigin(15.0f, 2.0f);
        strap.setPosition(5.0f, -13.0f);
        strap.setFillColor(sf::Color(105, 60, 30));
        window.draw(strap, states);

        // Left lens
        sf::CircleShape lens1(5.5f);
        lens1.setOrigin(5.5f, 5.5f);
        lens1.setPosition(3.0f, -13.0f);
        lens1.setFillColor(sf::Color(140, 240, 255));
        lens1.setOutlineColor(sf::Color(190, 140, 45));
        lens1.setOutlineThickness(2.0f);
        window.draw(lens1, states);

        // Right lens
        sf::CircleShape lens2(5.5f);
        lens2.setOrigin(5.5f, 5.5f);
        lens2.setPosition(13.0f, -13.0f);
        lens2.setFillColor(sf::Color(140, 240, 255));
        lens2.setOutlineColor(sf::Color(190, 140, 45));
        lens2.setOutlineThickness(2.0f);
        window.draw(lens2, states);

        // Lens specular sheen
        sf::CircleShape shine(1.8f);
        shine.setOrigin(1.8f, 1.8f);
        shine.setFillColor(sf::Color(255, 255, 255));
        shine.setPosition(1.5f, -14.5f);
        window.draw(shine, states);
        shine.setPosition(11.5f, -14.5f);
        window.draw(shine, states);
        break;
    }
    case HatId::TopHat: {
        // Brim
        sf::RectangleShape brim(sf::Vector2f(32.0f, 4.0f));
        brim.setOrigin(16.0f, 2.0f);
        brim.setPosition(5.0f, -16.0f);
        brim.setFillColor(sf::Color(24, 20, 32));
        brim.setOutlineColor(outlineColor);
        brim.setOutlineThickness(1.2f);
        window.draw(brim, states);

        // Cylinder body
        sf::RectangleShape crown(sf::Vector2f(22.0f, 22.0f));
        crown.setOrigin(11.0f, 22.0f);
        crown.setPosition(5.0f, -16.0f);
        crown.setFillColor(sf::Color(35, 30, 48));
        crown.setOutlineColor(outlineColor);
        crown.setOutlineThickness(1.2f);
        window.draw(crown, states);

        // Crimson silk ribbon band
        sf::RectangleShape band(sf::Vector2f(22.0f, 5.0f));
        band.setOrigin(11.0f, 5.0f);
        band.setPosition(5.0f, -16.0f);
        band.setFillColor(sf::Color(230, 45, 55));
        window.draw(band, states);
        break;
    }
    case HatId::CyberVisor: {
        // Futuristic glowing neon cyan visor
        sf::ConvexShape visor;
        visor.setPointCount(4);
        visor.setPoint(0, sf::Vector2f(-4.0f, -10.0f));
        visor.setPoint(1, sf::Vector2f(19.0f, -8.0f));
        visor.setPoint(2, sf::Vector2f(17.0f, -2.0f));
        visor.setPoint(3, sf::Vector2f(-2.0f, -4.0f));
        visor.setFillColor(sf::Color(0, 245, 255, 215));
        visor.setOutlineColor(sf::Color(255, 0, 180));
        visor.setOutlineThickness(1.5f);
        window.draw(visor, states);

        // Center laser grid line
        sf::RectangleShape laser(sf::Vector2f(21.0f, 1.8f));
        laser.setPosition(-3.0f, -6.5f);
        laser.setFillColor(sf::Color(255, 255, 255));
        window.draw(laser, states);
        break;
    }
    case HatId::RoyalCrown: {
        // Gleaming 3-peaked gold crown
        sf::ConvexShape crown;
        crown.setPointCount(7);
        crown.setPoint(0, sf::Vector2f(-9.0f, -16.0f));
        crown.setPoint(1, sf::Vector2f(-12.0f, -28.0f)); // Left peak
        crown.setPoint(2, sf::Vector2f(-3.0f, -22.0f));
        crown.setPoint(3, sf::Vector2f(4.0f, -32.0f));  // Center tallest peak
        crown.setPoint(4, sf::Vector2f(11.0f, -22.0f));
        crown.setPoint(5, sf::Vector2f(20.0f, -28.0f)); // Right peak
        crown.setPoint(6, sf::Vector2f(17.0f, -16.0f));
        crown.setFillColor(sf::Color(255, 215, 30));
        crown.setOutlineColor(outlineColor);
        crown.setOutlineThickness(1.5f);
        window.draw(crown, states);

        // Ruby and emerald jewels on peaks
        sf::CircleShape ruby(2.2f);
        ruby.setOrigin(2.2f, 2.2f);
        ruby.setFillColor(sf::Color(240, 45, 45));
        ruby.setPosition(4.0f, -31.0f);
        window.draw(ruby, states);

        sf::CircleShape gemL(1.8f);
        gemL.setOrigin(1.8f, 1.8f);
        gemL.setFillColor(sf::Color(45, 225, 120));
        gemL.setPosition(-12.0f, -27.0f);
        window.draw(gemL, states);
        gemL.setPosition(20.0f, -27.0f);
        window.draw(gemL, states);
        break;
    }
    case HatId::WizardHat: {
        // Pointed wizard hat
        sf::ConvexShape hatCone;
        hatCone.setPointCount(3);
        hatCone.setPoint(0, sf::Vector2f(-12.0f, -16.0f));
        hatCone.setPoint(1, sf::Vector2f(-18.0f, -38.0f)); // Bent tip
        hatCone.setPoint(2, sf::Vector2f(16.0f, -16.0f));
        hatCone.setFillColor(sf::Color(65, 35, 115));
        hatCone.setOutlineColor(outlineColor);
        hatCone.setOutlineThickness(1.5f);
        window.draw(hatCone, states);

        // Brim
        sf::RectangleShape brim(sf::Vector2f(34.0f, 4.0f));
        brim.setOrigin(17.0f, 2.0f);
        brim.setPosition(2.0f, -16.0f);
        brim.setFillColor(sf::Color(45, 20, 85));
        window.draw(brim, states);

        // Gold star
        sf::CircleShape star(3.0f, 5);
        star.setOrigin(3.0f, 3.0f);
        star.setFillColor(sf::Color(255, 235, 80));
        star.setPosition(0.0f, -25.0f);
        window.draw(star, states);
        break;
    }
    case HatId::PhoenixCrown: {
        // Blazing crown of fire flickers
        float p1 = std::sin(time * 16.0f) * 3.0f;
        float p2 = std::cos(time * 19.0f) * 4.0f;

        sf::ConvexShape fire;
        fire.setPointCount(5);
        fire.setPoint(0, sf::Vector2f(-8.0f, -15.0f));
        fire.setPoint(1, sf::Vector2f(-14.0f, -28.0f + p1));
        fire.setPoint(2, sf::Vector2f(4.0f, -36.0f + p2));
        fire.setPoint(3, sf::Vector2f(20.0f, -28.0f + p1));
        fire.setPoint(4, sf::Vector2f(15.0f, -15.0f));
        fire.setFillColor(sf::Color(255, 100, 20, 220));
        window.draw(fire, states);

        // Inner golden flame
        sf::ConvexShape innerFire;
        innerFire.setPointCount(3);
        innerFire.setPoint(0, sf::Vector2f(-2.0f, -15.0f));
        innerFire.setPoint(1, sf::Vector2f(4.0f, -30.0f + p2));
        innerFire.setPoint(2, sf::Vector2f(10.0f, -15.0f));
        innerFire.setFillColor(sf::Color(255, 235, 60));
        window.draw(innerFire, states);
        break;
    }
    case HatId::Astronaut: {
        // Clear space helmet bubble
        sf::CircleShape bubble(18.0f);
        bubble.setOrigin(18.0f, 18.0f);
        bubble.setPosition(4.0f, -8.0f);
        bubble.setFillColor(sf::Color(180, 235, 255, 95));
        bubble.setOutlineColor(sf::Color(245, 245, 255));
        bubble.setOutlineThickness(2.2f);
        window.draw(bubble, states);

        // Collar ring
        sf::RectangleShape collar(sf::Vector2f(26.0f, 4.0f));
        collar.setOrigin(13.0f, 2.0f);
        collar.setPosition(4.0f, 9.0f);
        collar.setFillColor(sf::Color(210, 215, 225));
        window.draw(collar, states);

        // Specular shine
        sf::CircleShape arc(8.0f);
        arc.setScale(1.0f, 0.4f);
        arc.setOrigin(8.0f, 8.0f);
        arc.setPosition(0.0f, -16.0f);
        arc.setFillColor(sf::Color(255, 255, 255, 180));
        window.draw(arc, states);
        break;
    }
    case HatId::CelestialHalo: {
        // Glowing platinum starlight halo floating above
        float floatOffset = std::sin(time * 6.0f) * 2.5f;

        // Outer glow
        sf::CircleShape glow(16.0f);
        glow.setScale(1.1f, 0.45f);
        glow.setOrigin(16.0f, 16.0f);
        glow.setPosition(5.0f, -27.0f + floatOffset);
        glow.setFillColor(sf::Color(140, 245, 255, 90));
        window.draw(glow, states);

        // Ring
        sf::CircleShape halo(13.0f);
        halo.setScale(1.1f, 0.4f);
        halo.setOrigin(13.0f, 13.0f);
        halo.setPosition(5.0f, -27.0f + floatOffset);
        halo.setFillColor(sf::Color::Transparent);
        halo.setOutlineColor(sf::Color(255, 255, 255));
        halo.setOutlineThickness(3.0f);
        window.draw(halo, states);
        break;
    }
    default:
        break;
    }
}

void CosmeticsManager::drawOutfit(sf::RenderWindow& window, const sf::Transform& birdTransform, OutfitId outfit, float time) {
    if (outfit == OutfitId::None) return;

    sf::RenderStates states;
    states.transform = birdTransform;

    switch (outfit) {
    case OutfitId::AviatorScarf: {
        // Flight scarf wrapped around neck
        sf::RectangleShape neck(sf::Vector2f(16.0f, 6.0f));
        neck.setOrigin(8.0f, 3.0f);
        neck.setPosition(4.0f, 6.0f);
        neck.setFillColor(sf::Color(235, 60, 50));
        neck.setOutlineColor(outlineColor);
        neck.setOutlineThickness(1.0f);
        window.draw(neck, states);

        // Fluttering tail
        float fl = std::sin(time * 20.0f) * 4.0f;
        sf::ConvexShape tail;
        tail.setPointCount(4);
        tail.setPoint(0, sf::Vector2f(-4.0f, 6.0f));
        tail.setPoint(1, sf::Vector2f(-22.0f, 9.0f + fl));
        tail.setPoint(2, sf::Vector2f(-24.0f, 15.0f + fl));
        tail.setPoint(3, sf::Vector2f(-2.0f, 11.0f));
        tail.setFillColor(sf::Color(215, 45, 35));
        window.draw(tail, states);
        break;
    }
    case OutfitId::DapperBowtie: {
        // Red silk bowtie
        sf::ConvexShape leftWing;
        leftWing.setPointCount(3);
        leftWing.setPoint(0, sf::Vector2f(8.0f, 7.0f));
        leftWing.setPoint(1, sf::Vector2f(2.0f, 3.0f));
        leftWing.setPoint(2, sf::Vector2f(2.0f, 11.0f));
        leftWing.setFillColor(sf::Color(220, 35, 45));
        window.draw(leftWing, states);

        sf::ConvexShape rightWing;
        rightWing.setPointCount(3);
        rightWing.setPoint(0, sf::Vector2f(8.0f, 7.0f));
        rightWing.setPoint(1, sf::Vector2f(14.0f, 3.0f));
        rightWing.setPoint(2, sf::Vector2f(14.0f, 11.0f));
        rightWing.setFillColor(sf::Color(220, 35, 45));
        window.draw(rightWing, states);

        // Center knot
        sf::CircleShape knot(2.5f);
        knot.setOrigin(2.5f, 2.5f);
        knot.setPosition(8.0f, 7.0f);
        knot.setFillColor(sf::Color(175, 20, 30));
        window.draw(knot, states);
        break;
    }
    case OutfitId::BikerVest: {
        // Leather biker vest
        sf::ConvexShape vest;
        vest.setPointCount(4);
        vest.setPoint(0, sf::Vector2f(-8.0f, -2.0f));
        vest.setPoint(1, sf::Vector2f(8.0f, 2.0f));
        vest.setPoint(2, sf::Vector2f(4.0f, 16.0f));
        vest.setPoint(3, sf::Vector2f(-12.0f, 12.0f));
        vest.setFillColor(sf::Color(35, 28, 42));
        vest.setOutlineColor(outlineColor);
        vest.setOutlineThickness(1.2f);
        window.draw(vest, states);

        // Silver zipper
        sf::RectangleShape zip(sf::Vector2f(1.8f, 12.0f));
        zip.setPosition(-2.0f, 2.0f);
        zip.setFillColor(sf::Color(220, 225, 235));
        window.draw(zip, states);
        break;
    }
    case OutfitId::RoyalCape: {
        // Flowing crimson velvet cape billowing
        float fl = std::sin(time * 14.0f) * 4.0f;
        sf::ConvexShape cape;
        cape.setPointCount(4);
        cape.setPoint(0, sf::Vector2f(-4.0f, -3.0f));
        cape.setPoint(1, sf::Vector2f(-30.0f, 8.0f + fl));
        cape.setPoint(2, sf::Vector2f(-26.0f, 22.0f + fl));
        cape.setPoint(3, sf::Vector2f(-6.0f, 10.0f));
        cape.setFillColor(sf::Color(185, 25, 35));
        cape.setOutlineColor(outlineColor);
        cape.setOutlineThickness(1.0f);
        window.draw(cape, states);

        // Gold clasp
        sf::CircleShape clasp(3.0f);
        clasp.setOrigin(3.0f, 3.0f);
        clasp.setPosition(-4.0f, -1.0f);
        clasp.setFillColor(sf::Color(255, 215, 30));
        window.draw(clasp, states);
        break;
    }
    case OutfitId::ArcaneRobe: {
        // Starry purple cowl
        sf::ConvexShape cowl;
        cowl.setPointCount(4);
        cowl.setPoint(0, sf::Vector2f(-6.0f, -4.0f));
        cowl.setPoint(1, sf::Vector2f(7.0f, -1.0f));
        cowl.setPoint(2, sf::Vector2f(3.0f, 15.0f));
        cowl.setPoint(3, sf::Vector2f(-14.0f, 12.0f));
        cowl.setFillColor(sf::Color(75, 35, 125));
        window.draw(cowl, states);

        // Gold trim
        sf::RectangleShape trim(sf::Vector2f(16.0f, 2.0f));
        trim.setPosition(-8.0f, 13.0f);
        trim.setFillColor(sf::Color(255, 225, 80));
        window.draw(trim, states);
        break;
    }
    case OutfitId::PhoenixWings: {
        // Flame plumage behind wing
        float fl = std::sin(time * 22.0f) * 5.0f;
        sf::ConvexShape flameTrail;
        flameTrail.setPointCount(3);
        flameTrail.setPoint(0, sf::Vector2f(-12.0f, -4.0f));
        flameTrail.setPoint(1, sf::Vector2f(-34.0f, -1.0f + fl));
        flameTrail.setPoint(2, sf::Vector2f(-14.0f, 8.0f));
        flameTrail.setFillColor(sf::Color(255, 120, 20, 190));
        window.draw(flameTrail, states);
        break;
    }
    case OutfitId::SpaceJetpack: {
        // Twin thruster rocket canisters
        sf::RectangleShape pack(sf::Vector2f(10.0f, 18.0f));
        pack.setOrigin(5.0f, 9.0f);
        pack.setPosition(-16.0f, 4.0f);
        pack.setFillColor(sf::Color(190, 195, 205));
        pack.setOutlineColor(outlineColor);
        pack.setOutlineThickness(1.2f);
        window.draw(pack, states);

        // Cyan rocket exhaust flicker
        float flame = std::sin(time * 30.0f) * 3.0f;
        sf::ConvexShape plume;
        plume.setPointCount(3);
        plume.setPoint(0, sf::Vector2f(-19.0f, 13.0f));
        plume.setPoint(1, sf::Vector2f(-16.0f, 23.0f + flame));
        plume.setPoint(2, sf::Vector2f(-13.0f, 13.0f));
        plume.setFillColor(sf::Color(0, 240, 255));
        window.draw(plume, states);
        break;
    }
    case OutfitId::CelestialMantle: {
        // Angelic starlight wings
        float fl = std::sin(time * 12.0f) * 3.5f;
        sf::ConvexShape wingL;
        wingL.setPointCount(4);
        wingL.setPoint(0, sf::Vector2f(-6.0f, -2.0f));
        wingL.setPoint(1, sf::Vector2f(-32.0f, -16.0f + fl));
        wingL.setPoint(2, sf::Vector2f(-26.0f, 2.0f + fl));
        wingL.setPoint(3, sf::Vector2f(-8.0f, 6.0f));
        wingL.setFillColor(sf::Color(220, 250, 255, 190));
        window.draw(wingL, states);
        break;
    }
    default:
        break;
    }
}
