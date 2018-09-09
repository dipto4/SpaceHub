

#ifndef POSTNEWTONIAN_H
#define POSTNEWTONIAN_H

#include "../type_class.h"
#include "../macros.h"
#include "../own_math.h"

namespace SpaceH {

    namespace Const {
        constexpr double INV_C = 1 / C;
        constexpr double INV_C2 = INV_C * INV_C;
        constexpr double INV_C3 = INV_C2 * INV_C;
        constexpr double INV_C4 = INV_C3 * INV_C;
        constexpr double INV_C5 = INV_C4 * INV_C;
    }


    template<typename TypeClass>
    struct NewtonianForce {
        /* Typedef */
        using type        = TypeClass;
        using Scalar      = typename type::Scalar;
        using Vector      = typename type::Vector;
        using VectorArray = typename type::VectorArray;
        /* Typedef */

        template <typename Particles>
        inline void operator()(const Particles& partc, VectorArray & acc) {

            for (auto& a : acc)
                a.setZero();

            size_t size = partc.particleNumber();
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = i + 1; j < size; ++j) {
                    Vector dr = partc.pos(j) - partc.pos(i);
                    Scalar re_r = dr.reNorm();
                    Scalar re_r3 = re_r * re_r * re_r;
                    acc[i] += dr * (re_r3 * partc.mass(j));
                    acc[j] -= dr * (re_r3 * partc.mass(i));
                }
            }
        }
    };


    template<typename TypeClass>
    struct KarmackNewtonian {
        /* Typedef */
        using type    = typename TypeClass::type;
        using Scalar  = typename type::Scalar;
        using Vector  = typename type::Vector;
        using VectorArray = typename type::VectorArray;
        using Buildin = typename  SpaceH::get_value_type<Scalar>::type;
        /* Typedef */

        template<typename Particles>
        inline void operator()(const Particles& partc, VectorArray & acc) {

            for (auto& a : acc)
                a.setZero();

            size_t size = partc.particleNumber();
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = i + 1; j < size; ++j) {
                    Vector dr = partc.pos(j) - partc.pos(i);
                    Scalar re_r = KarmackFastInverseSquareRoot<Buildin>(dr.norm2());
                    Scalar re_r3 = re_r * re_r * re_r;
                    acc[i] += dr * (re_r3 * partc.mass(j));
                    acc[j] -= dr * (re_r3 * partc.mass(i));
                }
            }
        }
    };


    /**
     *
     * @tparam TypeClass
     * @tparam First
     * @tparam Second
     * @tparam Radiative
     */
    template<typename TypeClass, bool First = true, bool Second = false, bool Radiative = false>
    class PostNewtonianForce {
    public:
        /* Typedef */
        using type        = typename TypeClass::type;
        using Scalar      = typename type::Scalar;
        using Vector      = typename type::Vector;
        using VectorArray = typename type::VectorArray;
        /* Typedef */

        template <typename  Particles>
        inline void operator()(const Particles& partc, VectorArray & acc) {

            for (auto &a : acc)
                a.setZero();
            
            if constexpr (Radiative) {
                RadiativeOrder(partc, acc);
            }
            if constexpr (Second) {
                SecondOrder(partc, acc);
            }
            if constexpr (First) {
                FirstOrder(partc, acc);
            }
        }
    private:
        template <typename  Particles>
        inline void FirstOrder(const Particles& partc, VectorArray & acc) {
            size_t size = partc.particleNumber();
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = i + 1; j < size; ++j) {
                    Vector dr = partc.pos(j) - partc.pos(i);
                    Vector dv = partc.vel(j) - partc.vel(i);

                }
            }
        }

        template <typename  Particles>
        inline void SecondOrder(const Particles& partc, VectorArray & acc) {
            size_t size = partc.particleNumber();
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = i + 1; j < size; ++j) {
                    Vector dr = partc.pos(j) - partc.pos(i);
                    Vector dv = partc.vel(j) - partc.vel(i);

                }
            }
        }

        template <typename  Particles>
        inline void RadiativeOrder(const Particles& partc, VectorArray & acc) {
            size_t size = partc.particleNumber();
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = i + 1; j < size; ++j) {
                    Vector dr = partc.pos(j) - partc.pos(i);
                    Vector dv = partc.vel(j) - partc.vel(i);

                }
            }
        }
    };
}

#endif
