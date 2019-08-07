//
// Created by yihan on 2/25/19.
//

#ifndef SPACEHUB_CHAINSYSTEM_H
#define SPACEHUB_CHAINSYSTEM_H

#include "../core-computation.hpp"
#include "../dev-tools.hpp"
#include "../particle-system.hpp"
#include "chain.hpp"
#include <type_traits>

namespace space {

    /*---------------------------------------------------------------------------*\
        Class ChainSystem Declaration
    \*---------------------------------------------------------------------------*/
    template<typename Particles, typename Forces>
    class ChainSystem : public ParticleSystem<ChainSystem<Particles, Forces>> {
    public:
        //Type members
        SPACEHUB_USING_TYPE_SYSTEM_OF(Particles);

        using Base = ParticleSystem<ChainSystem<Particles, Forces>>;

        using Particle = typename Particles::Particle;

        //Constructors
        ChainSystem() = delete;

        ChainSystem(ChainSystem const &) = default;

        ChainSystem(ChainSystem &&) noexcept = default;

        ChainSystem &operator=(ChainSystem const &) = default;

        ChainSystem &operator=(ChainSystem &&) noexcept = default;

        template<typename STL>
        ChainSystem(Scalar t, STL const &AoS_ptc);

        //Public methods
        SPACEHUB_STD_ACCESSOR(auto, chain_pos, chain_pos_);

        SPACEHUB_STD_ACCESSOR(auto, chain_vel, chain_vel_);

        SPACEHUB_STD_ACCESSOR(auto, index, index_);

        //Friend functions
        template <typename P, typename F>
        friend std::ostream &operator<<(std::ostream &os, ChainSystem<P, F> const &ps);

        template <typename P, typename F>
        friend std::istream &operator>>(std::istream &is, ChainSystem<P, F> &ps);

    CRTP_impl:
        //CRTP implementation
        SPACEHUB_STD_ACCESSOR(auto, impl_mass, ptc_.mass());

        SPACEHUB_STD_ACCESSOR(auto, impl_idn, ptc_.idn());

        SPACEHUB_STD_ACCESSOR(auto, impl_pos, ptc_.pos());

        SPACEHUB_STD_ACCESSOR(auto, impl_vel, ptc_.vel());

        SPACEHUB_STD_ACCESSOR(auto, impl_time, ptc_.time());

        size_t impl_number() const;

        void impl_advance_time(Scalar dt);

        void impl_advance_pos(Coord const &velocity, Scalar step_size);

        void impl_advance_vel(Coord const &acceleration, Scalar step_size);

        void impl_evaluate_acc(Coord &acceleration) const;

        void impl_drift(Scalar step_size);

        void impl_kick(Scalar step_size);

        void impl_pre_iter_process();

        void impl_post_iter_process();

        template<typename STL>
        void impl_to_linear_container(STL &stl);

        template<typename STL>
        void impl_load_from_linear_container(STL const &stl);

    private:
        //Private methods
        void chain_advance(Coord &var, Coord &ch_var, Coord &ch_inc, Scalar step_size);

        void eval_vel_indep_acc();

        void kick_pseu_vel(Scalar step_size);

        void kick_real_vel(Scalar step_size);

        //Private members
        Particles ptc_;
        Forces forces_;

        Coord chain_pos_;
        Coord chain_vel_;
        Coord acc_;
        Coord chain_acc_;

        IdxArray index_;
        IdxArray new_index_;

        std::conditional_t<Forces::ext_vel_indep, Coord, Empty> ext_vel_indep_acc_;
        std::conditional_t<Forces::ext_vel_dep, Coord, Empty> ext_vel_dep_acc_;
        std::conditional_t<Forces::ext_vel_dep, Coord, Empty> aux_vel_;
        std::conditional_t<Forces::ext_vel_dep, Coord, Empty> chain_aux_vel_;
    };

    /*---------------------------------------------------------------------------*\
        Class ChainSystem Implementation
    \*---------------------------------------------------------------------------*/
    template<typename Particles, typename Forces>
    template<typename STL>
    ChainSystem<Particles, Forces>::ChainSystem(Scalar t, const STL &AoS_ptc)
            : ptc_(t, AoS_ptc),
              chain_pos_(AoS_ptc.size()),
              chain_vel_(AoS_ptc.size()),
              index_(AoS_ptc.size()),
              new_index_(AoS_ptc.size()),
              acc_(AoS_ptc.size()),
              chain_acc_(AoS_ptc.size()) {
        static_assert(is_container_v<STL>, "Only STL-like container can be used");
        Chain::calc_chain_index(ptc_.pos(), index_);
        Chain::calc_chain(ptc_.pos(), chain_pos(), index());
        Chain::calc_chain(ptc_.vel(), chain_vel(), index());

        if constexpr (Forces::ext_vel_indep) {
            ext_vel_indep_acc_.resize(AoS_ptc.size());
        }

        if constexpr (Forces::ext_vel_dep) {
            ext_vel_dep_acc_.resize(AoS_ptc.size());
            aux_vel_ = ptc_.vel();
            chain_aux_vel_ = chain_vel_;
        }
    }

    template<typename Particles, typename Forces>
    size_t ChainSystem<Particles, Forces>::impl_number() const {
        return ptc_.number();
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::impl_advance_time(Scalar dt) {
        ptc_.time() += dt;
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::impl_advance_pos(const Coord &velocity, Scalar step_size) {
        Chain::calc_chain(velocity, chain_vel(), index());
        chain_advance(ptc_.pos(), chain_pos(), chain_vel(), step_size);
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::impl_advance_vel(const Coord &acceleration, Scalar step_size) {
        Chain::calc_chain(acceleration, chain_acc_, index());
        chain_advance(ptc_.vel(), chain_vel(), chain_acc_, step_size);
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::impl_evaluate_acc(Coord &acceleration) const {
        forces_.eval_acc(*this, acceleration);
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::impl_drift(Scalar step_size) {
        ptc_.time() += step_size;
        chain_advance(ptc_.pos(), chain_pos(), chain_vel(), step_size);
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::impl_kick(Scalar step_size) {
        if constexpr (Forces::ext_vel_dep) {
            Scalar half_step = 0.5 * step_size;
            eval_vel_indep_acc();
            kick_pseu_vel(half_step);
            kick_real_vel(step_size);
            kick_pseu_vel(half_step);
        } else {
            forces_.eval_acc(*this, acc_);
            impl_advance_vel(acc_, step_size);
        }
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::impl_pre_iter_process() {
        if constexpr (Forces::ext_vel_dep) {
            aux_vel_ = ptc_.vel();
            chain_aux_vel_ = chain_vel_;
        }
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::impl_post_iter_process() {
        /*Chain::calc_chain_index(ptc_.pos(), new_index_);
        if (new_index_ != index_) {
            Chain::update_chain(chain_pos_, index_, new_index_);
            Chain::calc_cartesian(ptc_.mass(), chain_pos_, ptc_.pos(), new_index_);
            Chain::update_chain(chain_vel_, index_, new_index_);
            Chain::calc_cartesian(ptc_.mass(), chain_vel_, ptc_.vel(), new_index_);
            index_ = new_index_;
        }*/
    }

    template<typename Particles, typename Forces>
    template<typename STL>
    void ChainSystem<Particles, Forces>::impl_to_linear_container(STL &stl) {
        stl.clear();
        stl.reserve(impl_number() * 6 + 1);
        stl.emplace_back(impl_time());
        add_coords_to(stl, chain_pos_);
        add_coords_to(stl, chain_vel_);
    }

    template<typename Particles, typename Forces>
    template<typename STL>
    void ChainSystem<Particles, Forces>::impl_load_from_linear_container(const STL &stl) {
        auto i = stl.begin();
        impl_time() = *i, ++i;
        load_to_coords(i, chain_pos_);
        load_to_coords(i, chain_vel_);
        Chain::calc_cartesian(ptc_.mass(), chain_pos_, ptc_.pos(), index_);
        Chain::calc_cartesian(ptc_.mass(), chain_vel_, ptc_.vel(), index_);
    }

    template<typename Particles, typename Forces>
    std::istream &operator>>(std::istream &is, ChainSystem<Particles, Forces> &ps) {
        is >> ps.ptc_;
        return is;
    }

    template<typename Particles, typename Forces>
    std::ostream &operator<<(std::ostream &os, const ChainSystem<Particles, Forces> &ps) {
        os << ps.ptc_;
        return os;
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::chain_advance(Coord &var, Coord &ch_var, Coord &ch_inc, Scalar step_size) {
        calc::coord_advance(ch_var, ch_inc, step_size);
        Chain::calc_cartesian(ptc_.mass(), ch_var, var, index());
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::eval_vel_indep_acc() {
        forces_.eval_newtonian_acc(*this, acc_);
        if constexpr (Forces::ext_vel_indep) {
            forces_.eval_extra_vel_indep_acc(*this, ext_vel_indep_acc_);
            calc::coord_add(acc_, acc_, ext_vel_indep_acc_);
        }
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::kick_pseu_vel(Scalar step_size) {
        forces_.eval_extra_vel_dep_acc(*this, ext_vel_dep_acc_);
        calc::coord_add(acc_, acc_, ext_vel_dep_acc_);
        Chain::calc_chain(acc_, chain_acc_, index());
        chain_advance(aux_vel_, chain_aux_vel_, chain_acc_, step_size);
    }

    template<typename Particles, typename Forces>
    void ChainSystem<Particles, Forces>::kick_real_vel(Scalar step_size) {
        std::swap(aux_vel_, ptc_.vel());
        std::swap(chain_aux_vel_, chain_vel());
        forces_.eval_extra_vel_dep_acc(*this, ext_vel_dep_acc_);
        std::swap(aux_vel_, ptc_.vel());
        std::swap(chain_aux_vel_, chain_vel());

        calc::coord_add(acc_, acc_, ext_vel_dep_acc_);
        Chain::calc_chain(acc_, chain_acc_, index());
        chain_advance(ptc_.vel(), chain_vel_(), chain_acc_, step_size);
    }
}
#endif //SPACEHUB_ARCHAIN_H