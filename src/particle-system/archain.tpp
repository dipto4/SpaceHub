//
// Created by yihan on 3/8/19.
//

#ifndef SPACEHUB_ARCHAIN_H
#define SPACEHUB_ARCHAIN_H

#include "regu-system.tpp"
#include "chain.tpp"

namespace SpaceH {

    template<typename Particles, typename Interactions, ReguType ReguType>
    class ARchainSystem : public ParticleSystem<ARchainSystem<Particles, Interactions, ReguType>> {
    public:
        SPACEHUB_USING_TYPE_SYSTEM_OF(Particles);

        using Particle = typename Particles::Particle;

        SPACEHUB_STD_ACCESSOR(auto, impl_mass, ptc_.mass());

        SPACEHUB_STD_ACCESSOR(auto, impl_idn, ptc_.idn());

        SPACEHUB_STD_ACCESSOR(auto, impl_pos, ptc_.pos());

        SPACEHUB_STD_ACCESSOR(auto, impl_vel, ptc_.vel());

        SPACEHUB_STD_ACCESSOR(auto, impl_time, ptc_.time());

        SPACEHUB_STD_ACCESSOR(auto, chain_pos, chain_pos_);

        SPACEHUB_STD_ACCESSOR(auto, chain_vel, chain_vel_);

        SPACEHUB_STD_ACCESSOR(auto, index, index_);

        ARchainSystem() = delete;

        template<typename STL>
        ARchainSystem(Scalar t, STL const &ptc) : ptc_(t, ptc), regu_(ptc), chain_pos_(ptc.size()), chain_vel_(ptc.size()), index_(ptc.size()), new_index_(ptc.size()), acc_(ptc.size()) , newtonian_acc_(ptc.size()),chain_acc_(ptc.size()){

            chain::calc_chain_index(ptc_.pos(), index_);
            chain::coord_calc_chain(ptc_.pos(), chain_pos(), index());
            chain::coord_calc_chain(ptc_.vel(), chain_vel(), index());
            if constexpr (Interactions::has_extra_vel_indep_acc) {
                extra_vel_indep_acc_.resize(ptc.size());
            }

            if constexpr (Interactions::has_extra_vel_dep_acc) {
                extra_vel_dep_acc_.resize(ptc.size());
                aux_vel_ = ptc_.vel();
                chain_aux_vel_ = chain_vel_;
            }
        }

        size_t impl_number() const {
            return ptc_.number();
        }

        void impl_advance_time(Scalar step_size) {
            Scalar phy_time = regu_.eval_pos_phy_time(ptc_, step_size);
            ptc_.time() += phy_time;
        }

        void impl_advance_pos(Coord const &velocity, Scalar step_size) {
            Scalar phy_time = regu_.eval_pos_phy_time(ptc_, step_size);
            chain::coord_calc_chain(velocity, chain_vel(), index());
            chain_advance(ptc_.pos(), chain_pos(), chain_vel(), phy_time);
        }

        void impl_advance_vel(Coord const &acceleration, Scalar step_size) {
            Scalar phy_time = regu_.eval_vel_phy_time(ptc_, step_size);
            chain::coord_calc_chain(acceleration, chain_acc_, index());
            chain_advance(ptc_.vel(), chain_vel(), chain_acc_, phy_time);
        }

        void impl_evaluate_acc(Coord &acceleration) const {
            eom_.eval_acc(*this, acceleration);
        }

        void impl_drift(Scalar step_size) {
            Scalar phy_time = regu_.eval_pos_phy_time(ptc_, step_size);
            ptc_.time() += phy_time;
            chain_advance(ptc_.pos(), chain_pos(), chain_vel(), phy_time);
        }

        void impl_kick(Scalar step_size) {
            Scalar phy_time = regu_.eval_vel_phy_time(ptc_, step_size);
            Scalar half_time = 0.5 * phy_time;

            eval_vel_indep_acc();

            if constexpr (Interactions::has_extra_vel_dep_acc) {
                kick_pseu_vel(half_time);
                kick_real_vel(phy_time);
                kick_pseu_vel(half_time);
            } else {
                if constexpr (Interactions::has_extra_vel_indep_acc) {
                    Calc::coord_add(acc_, newtonian_acc_, extra_vel_indep_acc_);

                    chain_advance(ptc_.vel(), chain_vel(), acc_, half_time);
                    advance_omega(ptc_.vel(), newtonian_acc_, phy_time);
                    advance_bindE(ptc_.vel(), extra_vel_indep_acc_, phy_time);
                    chain_advance(ptc_.vel(), chain_vel(), acc_, half_time);
                } else {
                    chain_advance(ptc_.vel(), chain_vel(), newtonian_acc_, half_time);
                    advance_omega(ptc_.vel(), newtonian_acc_, phy_time);
                    chain_advance(ptc_.vel(), chain_vel(), newtonian_acc_, half_time);
                }
            }
        }

        void impl_pre_iter_process() {
            if constexpr (Interactions::has_extra_vel_dep_acc) {
                aux_vel_ = ptc_.vel();
                chain_aux_vel_ = chain_vel_;
            }
        }

        void impl_post_iter_process() {
            chain::calc_chain_index(ptc_.pos(), new_index_);
            if(new_index_ != index_){
                chain::update_chain(chain_pos_, index_, new_index_);
                chain::coord_calc_cartesian(chain_pos_, ptc_.pos(), new_index_);
                Calc::coord_move_to_com(ptc_.mass(), ptc_.pos());
                chain::update_chain(chain_vel_, index_, new_index_);
                chain::coord_calc_cartesian(chain_vel_, ptc_.vel(), new_index_);
                Calc::coord_move_to_com(ptc_.mass(), ptc_.vel());
                index_ = new_index_;
            }
        }

        friend std::ostream &operator<<(std::ostream &os, ARchainSystem const &ps) {
            os << ps.ptc_;
            return os;
        }

        friend std::istream &operator>>(std::istream &is, ARchainSystem &ps) {
            is >> ps.ptc_;
            return is;
        }
    private:
        void chain_advance(Coord &var, Coord& ch_var, Coord & ch_inc, Scalar step_size) {
            Calc::coord_advance(ch_var, ch_inc, step_size);
            chain::coord_calc_cartesian(ch_var, var, index());
            Calc::coord_move_to_com(ptc_.mass(), var);
        }

        void eval_vel_indep_acc() {
            eom_.eval_newtonian_acc(ptc_, newtonian_acc_);
            if constexpr (Interactions::has_extra_vel_dep_acc) {
                eom_.eval_extra_vel_indep_acc(ptc_, extra_vel_indep_acc_);
            }
        }

        void advance_omega(Coord const &velocity, Coord const &d_omega_dr, Scalar phy_time) {
            Scalar d_omega = Calc::coord_contract_to_scalar(ptc_.mass(), velocity, d_omega_dr);
            regu_.omega() += d_omega * phy_time;
        }

        void advance_bindE(Coord const &velocity, Coord const &d_bindE_dr, Scalar phy_time) {
            Scalar d_bindE = -Calc::coord_contract_to_scalar(ptc_.mass(), velocity, d_bindE_dr);
            regu_.bindE() += d_bindE * phy_time;
        }

        void kick_pseu_vel(Scalar phy_time) {
            eom_.eval_extra_vel_dep_acc(ptc_, acc_.vel_dep_acc());
            Calc::coord_add(acc_, newtonian_acc_, extra_vel_dep_acc_);
            if constexpr (Interactions::has_extra_vel_indep_acc) {
                Calc::coord_add(acc_, acc_, extra_vel_indep_acc_);
            }
            chain_advance(aux_vel_, chain_aux_vel_, acc_, phy_time);
        }

        void kick_real_vel(Scalar phy_time) {
            std::swap(aux_vel_, ptc_.vel());
            std::swap(chain_aux_vel_, chain_vel());
            eom_.eval_extra_vel_dep_acc(ptc_, acc_.vel_dep_acc());
            std::swap(aux_vel_, ptc_.vel());
            std::swap(chain_aux_vel_, chain_vel());

            Calc::coord_add(acc_, newtonian_acc_, extra_vel_dep_acc_);
            if constexpr (Interactions::has_extra_vel_indep_acc) {
                Calc::coord_add(acc_, acc_, extra_vel_indep_acc_);
            }

            chain_advance(ptc_.vel(), chain_vel(), acc_, phy_time);

            advance_omega(aux_vel_, newtonian_acc_, phy_time);
            advance_bindE(aux_vel_, extra_vel_dep_acc_, phy_time);
        }


        Particles ptc_;
        Interactions eom_;
        Regularization<Scalar, ReguType> regu_;

        Coord chain_pos_{0};
        Coord chain_vel_{0};
        Coord acc_{0};
        Coord chain_acc_{0};

        Coord newtonian_acc_{0};
        Coord extra_vel_indep_acc_{0};
        Coord extra_vel_dep_acc_{0};
        Coord aux_vel_{0};
        Coord chain_aux_vel_{0};
        IdxArray index_{0};
        IdxArray new_index_{0};
    };
}
#endif //SPACEHUB_ARCHAIN_H