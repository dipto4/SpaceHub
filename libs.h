////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Filename:libs.h                                                                                                     //
//Author:Yihan Wang                                                                                                   //
//                                                                                                                    //
//                                                                                                                    //
//Description:                                                                                                        //
//                                                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef LIBS_H
#define LIBS_H
#include "vector3.h"

namespace SpaceH
{
    
/** @brief Self min()*/
template<typename T1, typename T2>
inline const T2 min(const T1& x, const T2& y)
{
    return x > y ? y : x;
}

/** @brief Self max()*/
template<typename T1, typename T2>
inline const T2 max(const T1& x, const T2& y)
{
    return y > x ? y : x;
}

/** @brief Self abs()*/
template<class T>
inline const T abs(const T& x)
{
    return x > -x ? x : -x;
}

/** @brief Self swap()*/
template <class T>
void swap(T& a, T& b)
{
    T tmp(std::move(a));
    a = std::move(b);
    b = std::move(tmp);
}

template<typename Scalar1,typename Scalar2>
inline void advanceScalar(Scalar1& var, Scalar2 increase)
{
    var += increase;
}

template<typename Scalar, typename Vector1, typename Vector2>
inline void advanceVector(Vector1& var, const Vector2& increase, Scalar stepSize )
{
    size_t size = var.size();
    for(size_t i = 0 ; i < size; i++)
    {
        var[i] += increase[i] * stepSize;
    }
}

/** @brief Move variables to central mass coordinates
 *
 *  @param mass   Array of mass.
 *  @param phyVar Array of variables need to be moved.
 */
template<typename Scalar, size_t N>
void MoveToCentralMassCoordinate(const std::array<Scalar, N>& mass, std::array<vec3<Scalar>, N>& phyVar)
{
    vec3<Scalar> centralMassVar(0.0, 0.0, 0.0);
    Scalar totalMass = 0;

    for(size_t i = 0 ; i < N ; ++i)
    {
        totalMass += mass[i];
        centralMassVar += phyVar[i] * mass[i];
    }

    centralMassVar /= totalMass;

    for(size_t i = 0 ; i < N ; ++i)
    {
        phyVar[i] -= centralMassVar;
    }
}

/** @brief Calculate the kinetic energy of particles
 *
 *  @param  mass          Array of mass.
 *  @param  vel           Array of velocity.
 *  @return The kinetic energy.
 */
template<typename Scalar, size_t N>
double getKineticEnergy(const std::array<Scalar, N>& mass, const std::array<vec3<Scalar>, N>& vel)
{
    Scalar kineticEnergy = 0;

    for(size_t i = 0 ;  i < N ; ++i)
        kineticEnergy += 0.5 * mass[i] * vel[i].normSquare();

    return kineticEnergy;
}

/** @brief Calculate the potential energy of particles
 *
 *  @param  mass            Array of mass.
 *  @param  pos             Array of position.
 *  @return The potential energy.
 */
template<typename Scalar, size_t N>
double getPotentialEnergy(const std::array<Scalar, N>& mass, const std::array<vec3<Scalar>, N>& pos)
{
    Scalar potentialEnergy = 0;

    for(size_t i = 0 ;  i < N ; ++i)
        for(size_t j = i + 1; j < N; ++j)
            potentialEnergy -= mass[i] * mass[j] / distance(pos[i], pos[j]);

    return potentialEnergy;
}

/** @brief Calculate the total(potential + kinetic) energy of particles
 *
 *  @param  mass            Array of mass.
 *  @param  pos             Array of position.
 *  @param  vel             Array of velocity.
 *  @return The total energy.
 */
template<typename Scalar, size_t N>
inline double getTotalEnergy(const std::array<Scalar, N>& mass, const std::array<vec3<Scalar>, N>& pos,
                             const std::array<vec3<Scalar>, N>& vel)
{
    Scalar potentialEnergy = 0;
    Scalar kineticEnergy   = 0;

    for(size_t i = 0 ;  i < N ; ++i)
    {
        kineticEnergy += 0.5 * mass[i] * vel[i].normSquare();

        for(size_t j = i + 1; j < N; ++j)
            potentialEnergy -= mass[i] * mass[j] / distance(pos[i], pos[j]);
    }

    return potentialEnergy + kineticEnergy;
}

/** @brief print an array. Used for debug*/
template<typename T>
void print(T& var)
{
    for(size_t i = 0 ; i < var.size(); ++i )
        std::cout << var[i] << ' ';

    std::cout << '\n';
}
    
}//end namespace SpaceH
#endif
