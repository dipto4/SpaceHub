
#ifndef DYNAMICCHAIN_H
#define DYNAMICCHAIN_H
#include "../../particles.h"
#include "chain.h"

namespace SpaceH
{
    
/**
 *  @brief Class of dynamical variable.
 *
 */
template<typename Dtype, size_t ArraySize, bool IsVelDep>
class VelIndepChainParticles : public ReguParticles<Dtype, ArraySize, IsVelDep>
{
public:
    /* Typedef */
    using Base = ReguParticles<Dtype, ArraySize, IsVelDep>;
    using typename Base::type;
    using Scalar       = typename type::Scalar;
    using Vector       = typename type::Vector;
    using VectorArray  = typename type::VectorArray;
    using IndexArray   = typename type::IndexArray;
    using ScalarBuffer = typename type::ScalarBuffer;
    /* Typedef */
    
    /*Template parameter check*/
    
    /*Template parameter check*/
    
    constexpr static SpaceH::DATASTRUCT dataStruct{SpaceH::DATASTRUCT::CHAIN};
    
    using Base::particleNumber;
    using Base::totalMass_;
    using Base::mass_;
    using Base::pos_;
    using Base::vel_;
    
    /** @brief Resize all containers if they are dynamical
     *  @param New size of container.
     */
    void resize(size_t new_siz)
    {
        static_cast<Base&>(*this).resize(new_siz);
        chain_pos_.resize(new_siz);
        chain_vel_.resize(new_siz);
        ch_index_.resize(new_siz);
    }
    
    /** @brief Reserve space for all containers if they are dynamical
     *  @param New capacity of container.
     */
    void reserve(size_t new_cap)
    {
        static_cast<Base&>(*this).reserve(new_cap);
        chain_pos_.reserve(new_cap);
        chain_vel_.reserve(new_cap);
        ch_index_.reserve(new_cap);
    }
    
    /**  @brief Position array const interface. Reference to pos_*/
    inline const VectorArray& chainPos() const { return chain_pos_; }
    
    /**  @brief Velocity array const interface. Reference to vel_*/
    inline const VectorArray& chainVel() const { return chain_vel_; }
    
    /**  @brief Index array const interface. Reference to ch_index_*/
    inline const IndexArray& chainIndex() const { return ch_index_; }
    
    /**  @brief Position vector const interface. Reference to pos_[i]*/
    inline const Vector& chainPos(size_t i) const { return chain_pos_[i]; }
    
    /**  @brief Velocity vecotr const interface. Reference to vel_[i]*/
    inline const Vector& chainVel(size_t i) const { return chain_vel_[i]; }
    
    /**  @brief Index const interface. Reference to ch_index_[i]*/
    inline const size_t chainIndex(size_t i) const { return ch_index_[i]; }
    
    
    /** @brief Advance the position array with internal velocity array.
     *  @param stepSize The advance step size.
     */
    inline void advancePos(Scalar stepSize)
    {
        SpaceH::advanceVector(chain_pos_, chain_vel_, stepSize);
        SpaceH::chain::synCartesian(chain_pos_, pos_, ch_index_);
        
        //SpaceH::advanceVector(pos_, vel_, stepSize);
        Vector CMPos = SpaceH::calcuCMCoord(mass_, pos_, totalMass_);
        SpaceH::moveToCMCoord(pos_, CMPos);
        
        
    }
    
    /** @brief Advance the  velocity array with given acceleration array.
     *  @param stepSize The advance step size.
     *  @param acc      The acceleration array.
     */
    inline void advanceVel(const VectorArray& acc, Scalar stepSize)
    {
        const size_t chain_num = this->particleNumber() - 1;
        
        VectorArray chain_acc;
        
        chain_acc.resize(chain_num + 1);
        
        for(size_t i = 0 ; i < chain_num; ++i)
            chain_acc[i] = acc[ch_index_[i + 1]] - acc[ch_index_[i]];
        
        chain_acc[chain_num].setZero();
        
        SpaceH::advanceVector(chain_vel_, chain_acc, stepSize);
        SpaceH::chain::synCartesian(chain_vel_, vel_, ch_index_);
        
        //SpaceH::advanceVector(vel_, acc, stepSize);
        Vector CMVel = SpaceH::calcuCMCoord(mass_, vel_, totalMass_);
        SpaceH::moveToCMCoord(vel_, CMVel);
        
    }
    
    /* Update the way to connect the chain if needed*/
    void updateChain()
    {
        
    }
    
    /** @brief Input(Initialize) variables with istream.*/
    friend std::istream& operator>>(std::istream& is, VelIndepChainParticles& partc)
    {
        is >> static_cast<Base&>(partc);
        
        /*  @note Due to the downcast here, this class has to maintain the size of the additional Arraies by itself.*/
        if(type::arraySize == SpaceH::DYNAMICAL)
        {
            size_t num = partc.idn_.size();
            partc.chain_pos_.resize(num);
            partc.chain_vel_.resize(num);
            partc.ch_index_.resize(num);
        }
        
        SpaceH::chain::getChainIndex(partc.pos_,  partc.ch_index_);
        SpaceH::chain::synChain(partc.pos_, partc.chain_pos_, partc.ch_index_);
        SpaceH::chain::synChain(partc.vel_, partc.chain_vel_, partc.ch_index_);
        
        return is;
    }
    
    /** @brief Input variables with plain scalar array.*/
    size_t read(const ScalarBuffer& data, const NbodyIO IO_flag = NbodyIO::STD)
    {
        size_t loc = static_cast<Base&>(*this).read(data, IO_flag);
        
        if(IO_flag == NbodyIO::ACTIVE)
        {
            size_t chain_num = particleNumber() - 1;
        
            for(size_t i = 0 ; i < chain_num ; ++i)
            {
                chain_pos_[i].x = data[loc++];
                chain_pos_[i].y = data[loc++];
                chain_pos_[i].z = data[loc++];
            }
        
            for(size_t i = 0 ; i < chain_num ; ++i)
            {
                chain_vel_[i].x = data[loc++];
                chain_vel_[i].y = data[loc++];
                chain_vel_[i].z = data[loc++];
            }
        }
        return loc;
    }
    
    /** @brief Output variables to plain scalar array.*/
    size_t write(ScalarBuffer& data, const NbodyIO IO_flag = NbodyIO::STD) const
    {
        size_t loc = static_cast<const Base&>(*this).write(data, IO_flag);
        
        if(IO_flag == NbodyIO::ACTIVE)
        {
            size_t chain_num  = particleNumber() - 1;
            
            data.reserve(loc + chain_num * 6);
            
            //for locality, split into two loops
            for(size_t i = 0; i < chain_num ; ++i)
            {
                data.emplace_back(chain_pos_[i].x);
                data.emplace_back(chain_pos_[i].y);
                data.emplace_back(chain_pos_[i].z);
            }
            
            for(size_t i = 0 ; i < chain_num ; ++i)
            {
                data.emplace_back(chain_vel_[i].x);
                data.emplace_back(chain_vel_[i].y);
                data.emplace_back(chain_vel_[i].z);
            }
        }
            return data.size();
    }

protected:
    /** @brief Chained position array of the particles. Element is 3D vector.*/
    VectorArray chain_pos_;
    
    /** @brief Chained velocity array of the particles. Element is 3D vector.*/
    VectorArray chain_vel_;
    
    /** @brief Index array from Cartesian coordinates to chain coordinates Element is 3D vector.*/
    IndexArray ch_index_;
};


template<typename Dtype, size_t ArraySize, bool IsVelDep>
class VelDepChainParticles : public VelIndepChainParticles<Dtype, ArraySize, IsVelDep>
{
public:
    /* Typedef */
    using Base = VelIndepChainParticles<Dtype, ArraySize, IsVelDep>;
    
    using typename Base::type;
    
    using Scalar = typename type::Scalar;
    
    using Vector = typename type::Vector;
    
    using VectorArray = typename type::VectorArray;
    
    using ScalarBuffer = typename type::ScalarBuffer;
    /* Typedef */
    
    /*Template parameter check*/
   
    /*Template parameter check*/
    
    using Base::particleNumber;
    using Base::totalMass_;
    using Base::mass_;
    using Base::vel_;
    using Base::chain_vel_;
    using Base::ch_index_;
    using Base::auxi_vel_;
    
    /** @brief Resize all containers if they are dynamical
     *  @param New size of container.
     */
    void resize(size_t new_siz)
    {
        static_cast<Base&>(*this).resize(new_siz);
        chain_auxi_vel_.resize(new_siz);
    }
    
    /** @brief Reserve space for all containers if they are dynamical
     *  @param New capacity of container.
     */
    void reserve(size_t new_cap)
    {
        static_cast<Base&>(*this).reserve(new_cap);
        chain_auxi_vel_.reserve(new_cap);
    }
    
    /**  @brief Velocity array const interface. Reference to vel_*/
    inline const VectorArray& chainAuxiVel() const { return chain_auxi_vel_; }
    
    /**  @brief Velocity vecotr const interface. Reference to vel_[i]*/
    inline const Vector& chainAuxiVel(size_t i) const { return chain_auxi_vel_[i]; }
    
    /** @brief Advance the  velocity array with given acceleration array.
     *  @param stepSize The advance step size.
     *  @param acc      The acceleration array.
     */
    inline void advanceAuxiVel(const VectorArray& acc, Scalar stepSize)
    {
        const size_t chain_num = this->particleNumber() - 1;
        
        VectorArray chain_acc;
        chain_acc.resize(chain_num + 1);
        
        
        for(size_t i = 0 ; i < chain_num; ++i)
            chain_acc[i] = acc[ch_index_[i + 1]] - acc[ch_index_[i]];
        
        chain_acc[chain_num].setZero();
        
        SpaceH::advanceVector(chain_auxi_vel_, chain_acc, stepSize);
        SpaceH::chain::synCartesian(chain_auxi_vel_, auxi_vel_, ch_index_);
        
        //SpaceH::advanceVector(auxi_vel_, acc, stepSize);
        
        Vector CMVel = SpaceH::calcuCMCoord(mass_, auxi_vel_, totalMass_);
        SpaceH::moveToCMCoord(auxi_vel_, CMVel);
        
    }
    
    /**  @brief synchronize auxiVel_ with vel_ */
    inline void synAuxiVelwithVel()
    {
        auxi_vel_ = vel_;
        chain_auxi_vel_ = chain_vel_;
    }
    
    /** @brief Input(Initialize) variables with istream.*/
    friend std::istream& operator>>(std::istream& is, VelDepChainParticles & partc)
    {
        is >> static_cast<Base&>(partc);
        
        partc.chain_auxi_vel_ = partc.chain_vel_;//assign here automatically resize the chain_auxi_vel_

        return is;
    }
    
    /** @brief Input variables with plain scalar array.*/
    size_t read(const ScalarBuffer& data, const NbodyIO IO_flag = NbodyIO::STD)
    {
        size_t loc = static_cast<Base&>(*this).read(data, IO_flag);
        
        if(IO_flag == NbodyIO::ACTIVE)
        {
            size_t chain_num = particleNumber() - 1;
        
            for(size_t i = 0 ; i < chain_num; ++i)
            {
                chain_auxi_vel_[i].x = data[loc++];
                chain_auxi_vel_[i].y = data[loc++];
                chain_auxi_vel_[i].z = data[loc++];
            }
        }
        return loc;
    }
    
    /** @brief Output variables to plain scalar array.*/
    size_t write(ScalarBuffer& data, const NbodyIO IO_flag = NbodyIO::STD) const
    {
        size_t loc = static_cast<const Base&>(*this).write(data, IO_flag);
        
        if(IO_flag == NbodyIO::ACTIVE)
        {
            size_t chain_num = particleNumber() - 1;
        
            data.reserve(loc + chain_num*3);
        
            for(size_t i = 0 ; i < chain_num; ++i)
            {
                data.emplace_back(chain_auxi_vel_[i].x);
                data.emplace_back(chain_auxi_vel_[i].y);
                data.emplace_back(chain_auxi_vel_[i].z);
            }
        }
        return data.size();
    }
    
private:
    /** @brief Chained velocity array of the particles. Element is 3D vector.*/
    VectorArray chain_auxi_vel_;
};
    
    template<typename Dtype, size_t ArraySize, bool IsVelDep>
    class ChainParticles : public VelIndepChainParticles<Dtype, ArraySize, IsVelDep> {};
    
    template<typename Dtype, size_t ArraySize>
    class ChainParticles<Dtype, ArraySize, true> : public VelDepChainParticles<Dtype, ArraySize, true> {};
}
#endif

