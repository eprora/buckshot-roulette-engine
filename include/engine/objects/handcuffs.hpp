#pragma once

#include <memory>

namespace engine{

    enum HandcuffType{
        None,
        Broken, // has ability to skip opponent move
        Intact // block adding handcuffs but have no ability
    };

    class Handcuffs {
    public:
        void add(){
            if(!isAllowedToAdd()) {
                throw std::logic_error("handcuffs are already in use and can't be used again");
            }
            type = Intact;
        }

        void remove() {
            type = None;
        }

        void decay(){
            switch (type) {
            case Intact:
                type = Broken;
                break;
            default:
                type = None;
                break;
            }
        }

        bool isAllowedToAdd() const{
            return type == None;
        }

        std::pair<std::bitset<32>, uint32_t> getHash() const{
            return {type == Broken? 0 : 1, 1};
        }

        bool operator==(const Handcuffs& other) const {
            if (isAllowedToAdd() != other.isAllowedToAdd()) {
                return false;
            }
            if (getHash() != other.getHash()) {
                return false;
            }
            return true;
        }
        
    private:
        HandcuffType type{None};
    };
}