
#ifndef IAS15ITERATOR_H
#define IAS15ITERATOR_H

#include "../dev-tools.hpp"
#include "../math.hpp"
#include "ode-iterator.hpp"
#include "../integrator/Gauss-Dadau.hpp"

namespace space::ode_iterator {

  /**
   * IAS15 iterator see details in https://arxiv.org/abs/1409.4779 .
   * @tparam Coords
   * @tparam ErrChecker
   * @tparam StepControl
   */
  template<typename Coords, template<typename> typename ErrChecker,
          template<size_t, typename> typename StepControl>
  class IAS15 : public OdeIterator<IAS15<Coords, ErrChecker, StepControl>> {
  public:
    using Scalar = typename Coords::Scalar;
    using Coord = Coords;
    using Integrator = integrator::GaussDadau<Coord>;

    IAS15() {
      PC_err_checker_.set_atol(0);
      PC_err_checker_.set_rtol(1e-16);
      err_checker_.set_atol(0);
      err_checker_.set_rtol(1e-9);
      step_controller_.set_safe_guards(0.95, 0.95, 0.02, 4);
    }

    template<typename U>
    auto impl_iterate(U &particles, typename U::Scalar macro_step_size) -> typename U::Scalar {
      Scalar iter_h = macro_step_size;
      integrator_.check_particle_size(particles.number());
      iter_table_ = integrator_.b_tab();
      for (size_t k = 0; k < max_iter_; ++k) {
        integrator_.calc_B_table(particles, iter_h);
        //Scalar error = err_checker_.error(integrator_.init_acc(), integrator_.b_tab()[6]);
        //space::std_print("stp error ",k, ' ', iter_h, ' ',error,"\n");
        if (in_converged_window()) {
          Scalar error = err_checker_.error(integrator_.init_acc(), integrator_.b_tab()[6]);
          Scalar new_iter_h = step_controller_.next_step_size((Integrator::order - 1) / 2, iter_h, error);
          if (error < static_cast<Scalar>(1)) {
            integrator_.integrate_to(particles, iter_h, Integrator::final_point);
            integrator_.predict_new_B(new_iter_h / iter_h);
            last_error_ = error;
            return new_iter_h;
          } else {//current step size is too large, restart the iteration with smaller iterH that has been determined by current error.
            integrator_.predict_new_B(new_iter_h / iter_h);
            iter_h = new_iter_h;
            k = 0;
          }
        }
        iter_table_ = integrator_.b_tab();
      }
      spacehub_abort("Exceed the max iteration number");
    }

  private:
    inline void reset_PC_iteration() {
      last_PC_error_ = math::max_value<Scalar>::value;
    }

    bool in_converged_window() {
      Scalar PC_error = PC_err_checker_.error(integrator_.init_acc(), iter_table_[6], integrator_.b_tab()[6]);
      if (PC_error < static_cast<Scalar>(1) || PC_error >= last_PC_error_) {
        reset_PC_iteration();
        return true;
      } else {
        last_PC_error_ = PC_error;
        return false;
      }
    }

    Integrator integrator_;
    typename Integrator::IterTable iter_table_;
    StepControl<7, Scalar> step_controller_;
    ErrChecker<Scalar> err_checker_;
    ErrChecker<Scalar> PC_err_checker_;
    Scalar last_PC_error_{math::max_value<Scalar>::value};
    Scalar last_error_{1};
    static constexpr size_t max_iter_{12};
  };
}
#endif