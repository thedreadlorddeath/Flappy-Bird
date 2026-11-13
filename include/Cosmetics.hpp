#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

enum class HatId {
    None = 0,
    Headband = 1,       // Score 1
    AviatorGoggles = 2, // Score 5
    TopHat = 3,         // Score 10
    CyberVisor = 4,     // Score 15
    RoyalCrown = 5,     // Score 20
    WizardHat = 6,      // Score 25
    PhoenixCrown = 7,   // Score 30
    Astronaut = 8,      // Score 35
    CelestialHalo = 9   // Score 40
};

enum class OutfitId {
    None = 0,
    AviatorScarf = 1,   // Score 5
    DapperBowtie = 2,   // Score 10
    BikerVest = 3,      // Score 15
    RoyalCape = 4,      // Score 20
    ArcaneRobe = 5,     // Score 25
    PhoenixWings = 6,   // Score 30
    SpaceJetpack = 7,   // Score 35
    CelestialMantle = 8 // Score 40
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

    std::string getHatName(HatId hat) const;
    std::string getOutfitName(OutfitId outfit) const;

    // Render cosmetic attachments anchored to the bird
    void drawHat(sf::RenderWindow& window, const sf::Transform& birdTransform, HatId hat, float time = 0.0f);
    void drawOutfit(sf::RenderWindow& window, const sf::Transform& birdTransform, OutfitId outfit, float time = 0.0f);

private:
    HatId m_equippedHat;
    OutfitId m_equippedOutfit;

    std::vector<CosmeticItem> m_hats;
    std::vector<CosmeticItem> m_outfits;
};
