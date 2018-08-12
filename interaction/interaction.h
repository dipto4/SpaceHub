
#ifndef INTERACTION_H
#define INTERACTION_H
#include "../protoType.h"
#include "force.h"
namespace SpaceH
{

template<typename VelIndep, typename VelDep = void, typename ExtVelIndep = void, typename ExtVelDep = void>
class Interactions
{
public:
    /* Typedef */
    using type        = typename VelIndep::type;
    using Scalar      = typename type::Scalar;
    using Vector      = typename type::Vector;
    using VectorArray = typename type::VectorArray;
    using ScalarArray = typename type::ScalarArray;
    /* Typedef */
    
    constexpr static bool isVelDep{ !std::is_void<VelDep>::value | !std::is_void<ExtVelDep>::value };
    
    const VectorArray& totalAcc() const { return acc_; }
    
    const VectorArray& velIndepAcc() const { return vel_indep_.acc(); }
    
    const VectorArray& velDepAcc() const { return vel_dep_.acc(); }
    
    const VectorArray& extVelIndepAcc() const { return ext_vel_indep_.acc(); }
    
    const VectorArray& extVelDepAcc() const { return ext_vel_dep_.acc(); }
    
    const Vector& totalAcc(size_t i) const { return acc_[i]; }
    
    const Vector& velIndepAcc(size_t i) const { return vel_indep_.acc(i); }
    
    const Vector& velDepAcc(size_t i) const { return vel_dep_.acc(i); }
    
    const Vector& extVelIndepAcc(size_t i) const { return ext_vel_indep_.acc(i); }
    
    const Vector& extVelDepAcc(size_t i) const { return ext_vel_dep_.acc(i); }
    
    
    template<typename Particles>
    inline typename std::enable_if<Particles::dataStruct == SpaceH::DATASTRUCT::PLAIN>::type
    calcuVelIndepAcc(const Particles& partc)
    {
        vel_indep_.calcuAcc(partc.mass(), partc.pos());
    }
    
    template<typename Particles>
    inline typename std::enable_if<Particles::dataStruct == SpaceH::DATASTRUCT::CHAIN>::type
    calcuVelIndepAcc(const Particles& partc)
    {
        vel_indep_.calcuAcc(partc.mass(), partc.pos(), partc.chainPos(), partc.chainIndex());
    }
    
    template<typename Particles>
    inline typename std::enable_if<Particles::dataStruct == SpaceH::DATASTRUCT::PLAIN>::type
    calcuVelDepAcc(const Particles& partc)
    {
        vel_dep_.calcuAcc(partc.mass(), partc.pos(), partc.vel());
    }
    
    template<typename Particles>
    inline typename std::enable_if<Particles::dataStruct == SpaceH::DATASTRUCT::CHAIN>::type
    calcuVelDepAcc(const Particles& partc)
    {
        vel_dep_.calcuAcc(partc.mass(), partc.pos(), partc.vel(), partc.chainPos(), partc.chainVel(), partc.chainIndex());
    }
    
    template<typename Particles>
    inline typename std::enable_if<Particles::dataStruct == SpaceH::DATASTRUCT::PLAIN>::type
    calcuAuxiVelDepAcc(const Particles& partc)
    {
        vel_dep_.calcuAcc(partc.mass(), partc.pos(), partc.auxiVel());
    }
    
    template<typename Particles>
    inline typename std::enable_if<Particles::dataStruct == SpaceH::DATASTRUCT::CHAIN>::type
    calcuAuxiVelDepAcc(const Particles& partc)
    {
        vel_dep_.calcuAcc(partc.mass(), partc.pos(), partc.auxiVel(), partc.chainPos(), partc.chainAuxiVel(), partc.chainIndex());
    }
    
    template<typename Particles>
    void calcuExtVelIndepAcc(const Particles& partc)
    {
        ext_vel_indep_.calcuAcc(partc.mass(), partc.pos());
    }
    
    template<typename Particles>
    void calcuExtVelDepAcc(const Particles& partc)
    {
        ext_vel_dep_.calcuAcc(partc.mass(), partc.pos(), partc.vel());
    }
    
    template<typename Particles>
    void calcuExtAuxiVelDepAcc(const Particles& partc)
    {
        ext_vel_dep_.calcuAcc(partc.mass(), partc.pos(), partc.auxiVel());
    }
    
    void zeroTotalAcc()
    {
        size_t size = acc_.size();
        for(size_t i = 0 ; i < size ; ++i)
            acc_[i].setZero();
    }
    
    void calcuTotalAcc()
    {
        acc_ = vel_indep_.acc();
        
        vel_dep_.addTotal(acc_);
        
        ext_vel_indep_.addTotal(acc_);
        
        ext_vel_dep_.addTotal(acc_);
    }
private:
    VectorArray acc_;
    
    SpaceH::VelIndepForce<VelIndep, Scalar, type::arraySize>       vel_indep_;
    
    SpaceH::VelDepForce<VelDep, Scalar, type::arraySize>           vel_dep_;
    
    SpaceH::ExtVelIndepForce<ExtVelIndep, Scalar, type::arraySize> ext_vel_indep_;
    
    SpaceH::ExtVelDepForce<ExtVelDep, Scalar, type::arraySize>     ext_vel_dep_;
};

    

    
}

#endif
