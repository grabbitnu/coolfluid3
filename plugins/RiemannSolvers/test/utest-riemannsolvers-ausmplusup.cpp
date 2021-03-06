// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::RiemannSolvers"

#include <iostream>
#include <boost/test/unit_test.hpp>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"
#include "RiemannSolvers/RiemannSolver.hpp"

#include "math/Defs.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::RiemannSolvers;
using namespace cf3::physics;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( RiemannSolvers_Suite )

BOOST_AUTO_TEST_CASE( NavierStokes2D_AUSMplusUp )
{
  Component& model =  *Core::instance().root().create_component<Component>("model2D");

  // Creation of physics + variables
  PhysModel& physics = *model.create_component("navierstokes2D","cf3.physics.NavierStokes.NavierStokes2D")->handle<PhysModel>();
  physics.options().set("gamma",1.4);
  physics.options().set("R", 287.05);
  Variables& sol_vars = *physics.create_variables("Cons2D","solution");

  // Creation + configuration of riemann solver
  RiemannSolver& riemann = *model.create_component("riemann","cf3.RiemannSolvers.AUSMplusUp")->handle<RiemannSolver>();
  riemann.options().set("machinf", 0.5);
  riemann.options().set("Ku", 0.75);
  riemann.options().set("Kp", 0.25);
  riemann.options().set("sigma", 1.);
  riemann.options().set("beta", 1./8.);
  riemann.options().set("physical_model",physics.handle<Component>());
  riemann.options().set("solution_vars",sol_vars.handle<Component>());

  std::cout << model.tree() << std::endl;

  // Check simple flux computation
  Uint dim  = physics.ndim();
  Uint neqs = physics.neqs();
  RealVector normal(dim);
  RealVector left(neqs), right(neqs);
  RealVector flux(neqs);
  RealVector wave_speeds(neqs);


  normal << 1., 0.;
  left << 4., 1., 0, 1000000.;
  right << 1., 0.5, 0., 250000.;

  riemann.compute_interface_flux_and_wavespeeds(left,right, normal, normal, flux, wave_speeds);

  std::cout << flux << "\n\n";

//  const Real tol (0.000001);
//  BOOST_CHECK_CLOSE(flux[0] ,1. , tol);

//  BOOST_CHECK_CLOSE(wave_speeds[0],  1. , tol);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

