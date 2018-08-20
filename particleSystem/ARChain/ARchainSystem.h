
#ifndef ARCHAIN_H
#define ARCHAIN_H
#include "../reguSystem/reguSystem.h"

namespace SpaceH
{
    
/**  @brief Algorithmatic Regularization chain System.
 *
 *   See details in https://arxiv.org/abs/0709.3367 .
 */
template <typename Particles, typename Interaction, typename Regularitor>
class ARchainSystem : public ReguSystem<Particles, Interaction, Regularitor>
{
public:
    using Base = ReguSystem<Particles, Interaction, Regularitor>;
    using type = typename Base::type;
    using Scalar = typename type::Scalar;
    using Base::partc;
    /*Template parameter check*/
    CHECK_TYPE(Particles, Interaction)
    CHECK_TYPE(Particles, Regularitor)
    /*Template parameter check*/
    
    void afterIterProcess()
    {
        this->partc.updateChain();
        Base::afterIterProcess();
    }
    
    /** @brief Calculate the potential energy of the system*/
    inline Scalar potentialEnergy()
    {
        return SpaceH::getPotentialEnergy(partc.mass(), partc.pos(), partc.chainPos(), partc.chainIndex());
    }

    /** @brief Calculate the total energy of the system*/
    inline Scalar totalEnergy()
    {
        return potentialEnergy() + this->kineticEnergy();
    }
    
};
    
}
#endif

