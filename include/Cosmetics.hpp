#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

enum class HatId {
    None = 0,
    Headband = 1,          // Score 1
    AviatorGoggles = 2,    // Score 15
    TopHat = 3,            // Score 35
    CyberVisor = 4,        // Score 65
    RoyalCrown = 5,        // Score 100
    WizardHat = 6,         // Score 150
    PhoenixCrown = 7,      // Score 220
    Astronaut = 8,         // Score 300
    CelestialHalo = 9,     // Score 400
    VoidHorns = 10,        // Score 500
    SolarDiadem = 11,      // Score 750
    OmnipotentCrown = 12   // Score 999 (Finale)
};

enum class OutfitId {
    None = 0,
    AviatorScarf = 1,      // Score 15
    DapperBowtie = 2,      // Score 35
    BikerVest = 3,         // Score 65
    RoyalCape = 4,         // Score 100
    ArcaneCowl = 5,        // Score 150
    PhoenixPlumes = 6,     // Score 220
    SpaceJetpack = 7,      // Score 300
    CelestialMantle = 8,   // Score 400
    ShadowWings = 9,       // Score 500
    CoronaWings = 10,      // Score 750
    SupernovaAura = 11     // Score 999 (Finale)
};

struct CosmeticItem {
    int id;
    std::string name;
    std::string description;
    int requiredScore;
};

class CosmeticsManager {
public:
    CosmeticsManager();

    void load();
    void save();

    HatId getEquippedHat() const { return m_equippedHat; }
    OutfitId getEquippedOutfit() const { return m_equippedOutfit; }

    void setEquippedHat(HatId hat);
    void setEquippedOutfit(OutfitId outfit);

    bool isHatUnlocked(HatId hat, int bestScore) const;
    bool isOutfitUnlocked(OutfitId outfit, int bestScore) const;

    void cycleNextHat(int bestScore);
    void cyclePrevHat(int bestScore);
    void cycleNextOutfit(int bestScore);
    void cyclePrevOutfit(int bestScore);

    const std::vector<CosmeticItem>& getHats() const { return m_hats; }
    const std::vector<CosmeticItem>& getOutfits() const { return m_outfits; }

    std::string getHatName(HatId hat) const {
        int idx = static_cast<int>(hat);
        if (idx >= 0 && idx < static_cast<int>(m_hats.size())) return m_hats[idx].name;
        return "Unknown";
    }

    std::string getOutfitName(OutfitId outfit) const {
        int idx = static_cast<int>(outfit);
        if (idx >= 0 && idx < static_cast<int>(m_outfits.size())) return m_outfits[idx].name;
        return "Unknown";
    }

    // Visual Rendering
    void drawHat(sf::RenderWindow& window, sf::Vector2f headPos, float angle, float scale, HatId hat, float time) const;
    void drawOutfit(sf::RenderWindow& window, sf::Vector2f bodyPos, float angle, float scale, OutfitId outfit, float time) const;
    void drawAuras(sf::RenderWindow& window, sf::Vector2f bodyPos, OutfitId outfit, HatId hat, float time) const;

    // Score 999 Apotheosis Visual Mode
    bool isApotheosisMode() const { return m_apotheosisMode; }
    bool isApotheosisModeEnabled() const { return m_apotheosisMode; }
    void toggleApotheosisMode(int bestScore) {
        if (bestScore >= 999) {
            m_apotheosisMode = !m_apotheosisMode;
            save();
        }
    }

private:
    HatId m_equippedHat;
    OutfitId m_equippedOutfit;
    bool m_apotheosisMode;

    std::vector<CosmeticItem> m_hats;
    std::vector<CosmeticItem> m_outfits;
};
