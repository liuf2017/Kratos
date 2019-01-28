from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7
# importing the Kratos Library
from KratosMultiphysics import *
from KratosMultiphysics.FluidDynamicsApplication import *
from KratosMultiphysics.ULFApplication import *
from KratosMultiphysics.MeshingApplication import *
from KratosMultiphysics.ConvectionDiffusionApplication import *
from KratosMultiphysics.ExternalSolversApplication import *
# Check that KratosMultiphysics was imported in the main script
CheckForPreviousImport()

variables_dictionary = {"PRESSURE" : PRESSURE,
                        "VELOCITY" : VELOCITY,
                        "REACTION" : REACTION,
                        "DISTANCE" : DISTANCE,
			 "AUX_VEL" : AUX_VEL,                        
                        "DISPLACEMENT" : DISPLACEMENT,
                        "IS_INTERFACE" : IS_INTERFACE,
                        "IS_STRUCTURE" : IS_STRUCTURE,
                        "VISCOUS_STRESSX": VISCOUS_STRESSX,
                        "VISCOUS_STRESSY": VISCOUS_STRESSY,
                        "IS_WATER": IS_WATER,
                        "DENSITY": DENSITY,
                        "VISCOSITY": VISCOSITY,
                        "TEMPERATURE": TEMPERATURE}


def AddVariables(model_part, config=None):
    model_part.AddNodalSolutionStepVariable(VELOCITY)
    model_part.AddNodalSolutionStepVariable(ACCELERATION)
    model_part.AddNodalSolutionStepVariable(MESH_VELOCITY)
    model_part.AddNodalSolutionStepVariable(PRESSURE)
    model_part.AddNodalSolutionStepVariable(IS_STRUCTURE)
    model_part.AddNodalSolutionStepVariable(IS_STRUCTURE)
    model_part.AddNodalSolutionStepVariable(DISPLACEMENT)
    model_part.AddNodalSolutionStepVariable(VISCOSITY)
    model_part.AddNodalSolutionStepVariable(DENSITY)
    model_part.AddNodalSolutionStepVariable(BODY_FORCE)
    model_part.AddNodalSolutionStepVariable(NODAL_AREA)
    model_part.AddNodalSolutionStepVariable(NODAL_H)
    model_part.AddNodalSolutionStepVariable(NODAL_LENGTH)
    model_part.AddNodalSolutionStepVariable(ADVPROJ)
    model_part.AddNodalSolutionStepVariable(DIVPROJ)
    model_part.AddNodalSolutionStepVariable(REACTION)
    model_part.AddNodalSolutionStepVariable(REACTION_WATER_PRESSURE)
    model_part.AddNodalSolutionStepVariable(EXTERNAL_PRESSURE)
    model_part.AddNodalSolutionStepVariable(FLAG_VARIABLE)
    model_part.AddNodalSolutionStepVariable(NORMAL)
    model_part.AddNodalSolutionStepVariable(Y_WALL)
    model_part.AddNodalSolutionStepVariable(PATCH_INDEX)
    model_part.AddNodalSolutionStepVariable(IS_FREE_SURFACE)
    #20150128 ajarauta: surface tension variables
    model_part.AddNodalSolutionStepVariable(MEAN_CURVATURE_2D)
    model_part.AddNodalSolutionStepVariable(IS_WATER)
    model_part.AddNodalSolutionStepVariable(VISCOUS_STRESSX)
    model_part.AddNodalSolutionStepVariable(VISCOUS_STRESSY)
    model_part.AddNodalSolutionStepVariable(VISCOUS_STRESSZ)
    model_part.AddNodalSolutionStepVariable(TRIPLE_POINT)
    model_part.AddNodalSolutionStepVariable(IS_INTERFACE)
    model_part.AddNodalSolutionStepVariable(FORCE)
    model_part.AddNodalSolutionStepVariable(IS_BOUNDARY)
    model_part.AddNodalSolutionStepVariable(IS_FLUID)
    model_part.AddNodalSolutionStepVariable(CONTACT_ANGLE)
    model_part.AddNodalSolutionStepVariable(FRACT_VEL)
    model_part.AddNodalSolutionStepVariable(IS_LAGRANGIAN_INLET)
    model_part.AddNodalSolutionStepVariable(MEAN_CURVATURE_3D)
    model_part.AddNodalSolutionStepVariable(GAUSSIAN_CURVATURE)
    model_part.AddNodalSolutionStepVariable(PRINCIPAL_CURVATURE_1)
    model_part.AddNodalSolutionStepVariable(PRINCIPAL_CURVATURE_2)
    model_part.AddNodalSolutionStepVariable(PRINCIPAL_DIRECTION_1)
    model_part.AddNodalSolutionStepVariable(PRINCIPAL_DIRECTION_2)
    model_part.AddNodalSolutionStepVariable(NORMAL_GEOMETRIC)
    model_part.AddNodalSolutionStepVariable(ADHESION_FORCE)
    model_part.AddNodalSolutionStepVariable(NORMAL_CONTACT_LINE)
    model_part.AddNodalSolutionStepVariable(NORMAL_CONTACT_LINE_EQUILIBRIUM)
    model_part.AddNodalSolutionStepVariable(NORMAL_EQUILIBRIUM)
    model_part.AddNodalSolutionStepVariable(NORMAL_TRIPLE_POINT)
    #model_part.AddNodalSolutionStepVariable(PHASE_FRACTION_GRADIENT)
    model_part.AddNodalSolutionStepVariable(DISSIPATIVE_FORCE_COEFF_JM_X)
    model_part.AddNodalSolutionStepVariable(DISSIPATIVE_FORCE_COEFF_BM_X)
    model_part.AddNodalSolutionStepVariable(DISSIPATIVE_FORCE_COEFF_SM_X)
    model_part.AddNodalSolutionStepVariable(DISSIPATIVE_FORCE_COEFF_JM_Y)
    model_part.AddNodalSolutionStepVariable(DISSIPATIVE_FORCE_COEFF_BM_Y)
    model_part.AddNodalSolutionStepVariable(DISSIPATIVE_FORCE_COEFF_SM_Y)
    model_part.AddNodalSolutionStepVariable(TEMPERATURE)
    model_part.AddNodalSolutionStepVariable(SURFACE_TENSION_COEF)


def AddDofs(model_part, config=None):
    for node in model_part.Nodes:
        # adding dofs
        node.AddDof(VELOCITY_X, REACTION_X)
        node.AddDof(VELOCITY_Y, REACTION_Y)
        node.AddDof(VELOCITY_Z, REACTION_Z)
        node.AddDof(PRESSURE, REACTION_WATER_PRESSURE)


    print("dofs for the SurfaceTension monolithic solver added correctly")


class STMonolithicSolver:
    def __init__(self, model_part, domain_size, eul_model_part, gamma, contact_angle, zeta_dissapative_JM_x, zeta_dissapative_BM_x, zeta_dissapative_SM_x, zeta_dissapative_JM_y, zeta_dissapative_BM_y, zeta_dissapative_SM_y,surface_temp):
        self.model_part = model_part
        self.domain_size = domain_size
        # eul_model_part can be 0 (meaning that the model part is lagrangian) or 1 (eulerian)
        self.eul_model_part = eul_model_part

        self.alpha = -0.3
        if(eul_model_part == 0):
            self.move_mesh_strategy = 2
        else:
            self.move_mesh_strategy = 0

        # definition of the solvers
        try:
            from KratosMultiphysics.ExternalSolversApplication import SuperLUIterativeSolver
            self.linear_solver = SuperLUIterativeSolver()
        except:
            self.linear_solver = SkylineLUFactorizationSolver()

        # definition of the convergence criteria
        self.rel_vel_tol = 1e-3
        self.abs_vel_tol = 1e-6
        self.rel_pres_tol = 1e-3
        self.abs_pres_tol = 1e-6
        self.dynamic_tau = 0.0
        self.oss_switch = 0

        # non newtonian setting
        self.regularization_coef = 1000
        self.max_iter = 30
        self.contact_angle = contact_angle
        self.gamma = gamma
        self.surface_temp = surface_temp
        self.zeta_dissapative_JM_x = zeta_dissapative_JM_x
        self.zeta_dissapative_BM_x = zeta_dissapative_BM_x
        self.zeta_dissapative_SM_x = zeta_dissapative_SM_x
        self.zeta_dissapative_JM_y = zeta_dissapative_JM_y
        self.zeta_dissapative_BM_y = zeta_dissapative_BM_y
        self.zeta_dissapative_SM_y = zeta_dissapative_SM_y
        #self.gamma_sl = gamma_sl
        #self.gamma_sv = gamma_sv
        

        # default settings
        self.echo_level = 0
        self.compute_reactions = True
        self.ReformDofSetAtEachStep = True
        self.CalculateNormDxFlag = True
        self.MoveMeshFlag = True
        self.use_slip_conditions = False

        #self.time_scheme = None
        #self.builder_and_solver = None

        self.turbulence_model = None
        self.use_spalart_allmaras = False
        self.use_des = False
        self.Cdes = 1.0
        self.wall_nodes = list()
        self.spalart_allmaras_linear_solver = None

        self.divergence_clearance_steps = 0

        print("Construction monolithic solver finished")

        print("after reading all the model contains:")
        print(self.model_part)

        if(self.eul_model_part == 0):
            self.UlfUtils = UlfUtils()
          #self.PfemUtils = PfemUtils()
            self.mark_outer_nodes_process = MarkOuterNodesProcess(model_part);
            self.node_erase_process = NodeEraseProcess(model_part);
            self.add_nodes=True
            self.alpha_shape = 3.5;
            self.mark_free_surface_process = MarkFreeSurfaceProcess(model_part);
            #self.mark_fluid_process = MarkFluidProcess(model_part);

	  #saving the limits of the box (all the nodes external to this will be erased)
            bounding_box_corner1_x = -1.00000e+00
            bounding_box_corner1_y = -1.00000e+00
            bounding_box_corner1_z = -1.00000e+00
            bounding_box_corner2_x = 1.01000e+10
            bounding_box_corner2_y = 1.01000e+10
            bounding_box_corner2_z = 1.01000e+10
            box_corner1 = Vector(3);
            box_corner1[0]=bounding_box_corner1_x; box_corner1[1]=bounding_box_corner1_y; box_corner1[2]=bounding_box_corner1_z;
            box_corner2 = Vector(3);
            box_corner2[0]=bounding_box_corner2_x; box_corner2[1]=bounding_box_corner2_y; box_corner2[2]=bounding_box_corner2_z;
            self.box_corner1 = box_corner1
            self.box_corner2 = box_corner2

            if(domain_size == 2):
                self.Mesher =  TriGenDropletModeler()
                self.fluid_neigh_finder = FindNodalNeighboursProcess(model_part,9,18)
                #this is needed if we want to also store the conditions a node belongs to
                self.condition_neigh_finder = FindConditionsNeighboursProcess(model_part,2, 10)

            (self.fluid_neigh_finder).Execute();
            Hfinder  = FindNodalHProcess(model_part);
            Hfinder.Execute();

    #
    def Initialize(self):

        if self.use_slip_conditions == False:
            for cond in self.model_part.Conditions:
                if cond.GetValue(IS_STRUCTURE) == 1.0:
                    self.use_slip_conditions = False
                    break

        # if we use slip conditions, calculate normals on the boundary
        if self.use_slip_conditions:
            self.normal_util = NormalCalculationUtils()
            self.normal_util.CalculateOnSimplex(
                self.model_part, self.domain_size, IS_STRUCTURE)

            for cond in self.model_part.Conditions:
                if cond.GetValue(IS_STRUCTURE) == 1.0:
                    for node in cond.GetNodes():
                        node.SetValue(IS_STRUCTURE, 1.0)

        # creating the solution strategy
        self.conv_criteria = VelPrCriteria(self.rel_vel_tol, self.abs_vel_tol, self.rel_pres_tol, self.abs_pres_tol)
        #self.conv_criteria = ResidualCriteria(0.0001, 0.0000001)

        #(self.conv_criteria).SetEchoLevel(self.echo_level)

        self.time_scheme = ResidualBasedPredictorCorrectorVelocityBossakSchemeTurbulent(self.alpha, self.move_mesh_strategy, self.domain_size)

        builder_and_solver = ResidualBasedBlockBuilderAndSolver(
            self.linear_solver)

        self.solver = ResidualBasedNewtonRaphsonStrategy(
            self.model_part, self.time_scheme, self.linear_solver, self.conv_criteria,
            builder_and_solver, self.max_iter, self.compute_reactions, self.ReformDofSetAtEachStep, self.MoveMeshFlag)
        (self.solver).SetEchoLevel(self.echo_level)
        self.solver.Check()

        self.model_part.ProcessInfo.SetValue(DYNAMIC_TAU, self.dynamic_tau)
        self.model_part.ProcessInfo.SetValue(OSS_SWITCH, self.oss_switch)
        self.model_part.ProcessInfo.SetValue(M, self.regularization_coef)

        self.model_part.ProcessInfo.SetValue(CONTACT_ANGLE_STATIC, self.contact_angle)
        self.model_part.ProcessInfo.SetValue(SURFACE_TENSION_COEF, self.gamma)
        self.model_part.ProcessInfo.SetValue(DISSIPATIVE_FORCE_COEFF_JM_X, self.zeta_dissapative_JM_x)
        self.model_part.ProcessInfo.SetValue(DISSIPATIVE_FORCE_COEFF_BM_X, self.zeta_dissapative_BM_x)
        self.model_part.ProcessInfo.SetValue(DISSIPATIVE_FORCE_COEFF_SM_X, self.zeta_dissapative_SM_x)
        self.model_part.ProcessInfo.SetValue(DISSIPATIVE_FORCE_COEFF_JM_Y, self.zeta_dissapative_JM_y)
        self.model_part.ProcessInfo.SetValue(DISSIPATIVE_FORCE_COEFF_BM_Y, self.zeta_dissapative_BM_y)
        self.model_part.ProcessInfo.SetValue(DISSIPATIVE_FORCE_COEFF_SM_Y, self.zeta_dissapative_SM_y)
        self.model_part.ProcessInfo.SetValue(TEMPERATURE, self.surface_temp)

        if(self.eul_model_part == 0):
	    #marking the fluid
            (self.fluid_neigh_finder).Execute();
            (self.mark_free_surface_process).Execute();
            if (self.domain_size == 2):
                FindTriplePoint().FindTriplePoint2D(self.model_part)
            self.Remesh()

# print "Initialization monolithic solver finished"
    #
    def Solve(self):

        if self.ReformDofSetAtEachStep:
            if self.use_slip_conditions:
                self.normal_util.CalculateOnSimplex(
                    self.model_part, self.domain_size, IS_STRUCTURE)
            if self.use_spalart_allmaras:
                self.neighbour_search.Execute()

        NormalCalculationUtils().CalculateOnSimplex(self.model_part.Conditions, self.domain_size)
        for node in self.model_part.Nodes:
            if (node.GetSolutionStepValue(IS_BOUNDARY) != 1.0):# and node.GetSolutionStepValue(TRIPLE_POINT) == 0):
                node.SetSolutionStepValue(NORMAL_X,0,0.0)
                node.SetSolutionStepValue(NORMAL_Y,0,0.0)
                node.SetSolutionStepValue(NORMAL_Z,0,0.0)
        if (self.domain_size == 2):
            FindTriplePoint().FindTriplePoint2D(self.model_part)
            self.temp()
            CalculateCurvature().CalculateCurvature2D(self.model_part)
            CalculateNodalLength().CalculateNodalLength2D(self.model_part)
            CalculateContactAngle().CalculateContactAngle2D(self.model_part)
            self.cont_angle_cond()
        #self.solver.MoveMesh()
        (self.solver).Solve() #it dumps in this line... 20151020
        #AssignPointNeumannConditions().AssignPointNeumannConditions3D(self.model_part)

        #self.solver.MoveMesh()

        if(self.eul_model_part == 0):
            #(self.fluid_neigh_finder).Execute();
            (self.fluid_neigh_finder).Execute();
            self.Remesh();


    def SetEchoLevel(self, level):
        (self.solver).SetEchoLevel(level)


    def Clear(self):
        (self.solver).Clear()



    ##########################################
    def Remesh(self):

        self.UlfUtils.MarkNodesTouchingWall(self.model_part, self.domain_size, 0.1)

        ##erase all conditions and elements prior to remeshing
        ((self.model_part).Elements).clear();

        (self.mark_outer_nodes_process).MarkOuterNodes(self.box_corner1, self.box_corner2);

        h_factor=0.25;

        if (self.domain_size == 2):
            (self.Mesher).ReGenerateMeshDroplet("SurfaceTension2D","Condition2D", self.model_part, self.node_erase_process, True, True, self.alpha_shape, h_factor)

        (self.fluid_neigh_finder).Execute();
        (self.condition_neigh_finder).Execute();

        #print "marking fluid" and applying fluid boundary conditions
        (self.mark_free_surface_process).Execute();
        #(self.mark_fluid_process).Execute();
        (self.UlfUtils).CalculateNodalArea(self.model_part,self.domain_size);
        self.UlfUtils.MarkLonelyNodesForErasing(self.model_part)
        self.node_erase_process.Execute()
        #(self.mark_fluid_process).Execute();
        self.UlfUtils.MarkLonelyNodesForErasing(self.model_part)#, self.domain_size)
        self.node_erase_process.Execute()

        ##############THIS IS FOR EMBEDDED"""""""""""""""""""""""""
######################################################################################################
        #FOR PFEM
        NormalCalculationUtils().CalculateOnSimplex(self.model_part.Conditions, self.domain_size)
        if (self.domain_size == 2):
            #for node in self.model_part.Nodes:
                #node.SetSolutionStepValue(FLAG_VARIABLE,0,0.0)
#
            for node in self.model_part.Nodes:
                if (node.GetSolutionStepValue(IS_FREE_SURFACE) > 0.99999999999999999999):
                    node.SetSolutionStepValue(FLAG_VARIABLE, 0, 1.0)
                    node.SetSolutionStepValue(IS_INTERFACE, 0, 1.0)
                else:
                    node.SetSolutionStepValue(IS_INTERFACE, 0, 0.0)
            for node in self.model_part.Nodes:
                if (node.GetSolutionStepValue(IS_BOUNDARY) != 1.0):# and node.GetSolutionStepValue(TRIPLE_POINT) == 0):
                    node.SetSolutionStepValue(NORMAL_X,0,0.0)
                    node.SetSolutionStepValue(NORMAL_Y,0,0.0)
                    node.SetSolutionStepValue(NORMAL_Z,0,0.0)
            FindTriplePoint().FindTriplePoint2D(self.model_part)
            self.temp()
            CalculateCurvature().CalculateCurvature2D(self.model_part)
            CalculateNodalLength().CalculateNodalLength2D(self.model_part)
            CalculateContactAngle().CalculateContactAngle2D(self.model_part)
            self.cont_angle_cond()

        ##############THIS IS FOR EMBEDDED"""""""""""""""""""""""""
        print("end of remesh function")
    ######################################################################

    def FindNeighbours(self):
        (self.neigh_finder).Execute();
        
    def temp(self):
        const = 1.0
        surface_temp = self.surface_temp
        gamma = self.gamma
        time = self.model_part.ProcessInfo.GetValue(TIME)
        dt   = self.model_part.ProcessInfo.GetValue(DELTA_TIME)
        #surface_temp = const * time * 100.0 + surface_temp
        for node in self.model_part.Nodes:
            if (node.GetSolutionStepValue(IS_BOUNDARY)!=0):
                node.SetSolutionStepValue(TEMPERATURE,0, surface_temp)
                node.Free(TEMPERATURE)
                gamma = 0.07275*(1.0-0.00212*(surface_temp-293.15))
                node.SetSolutionStepValue(SURFACE_TENSION_COEF,0, gamma)
                node.Free(SURFACE_TENSION_COEF)
                CalculateCurvature().CalculateCurvature2D(self.model_part)
                CalculateNodalLength().CalculateNodalLength2D(self.model_part)

    def cont_angle_cond(self):
        theta_adv = 108.0
        theta_rec = 108.0
        time = self.model_part.ProcessInfo.GetValue(TIME)
        dt   = self.model_part.ProcessInfo.GetValue(DELTA_TIME)
        if (dt>0):
            for node in self.model_part.Nodes:
                vc = Vector(2)
                vc[0]=node.GetSolutionStepValue(VELOCITY_X)
                vc[1]=node.GetSolutionStepValue(VELOCITY_Y)
                if (node.GetSolutionStepValue(IS_STRUCTURE) == 1.0):
                    vc[0] = 0.0
                    vc[1] = 0.0
                    node.SetSolutionStepValue(VELOCITY_Y,0, vc[0])
                    node.SetSolutionStepValue(VELOCITY_X,0, vc[1])
                    node.Fix(VELOCITY_X)
                    node.Fix(VELOCITY_Y)
                    node.Fix(IS_STRUCTURE)
                    if (node.GetSolutionStepValue(TRIPLE_POINT) == 1.0):
                        node.Free(DISPLACEMENT_X)
                if(node.GetSolutionStepValue(TRIPLE_POINT) == 1.0):
                    if((node.GetSolutionStepValue(CONTACT_ANGLE)> theta_adv) or (node.GetSolutionStepValue(CONTACT_ANGLE)< theta_rec)):
                        node.Free(VELOCITY_X)
                        a = node.GetSolutionStepValue(DISPLACEMENT_X,1)
                        b = node.GetSolutionStepValue(DISPLACEMENT_X,0)
                        c= b - a
                        vc[0] = c/dt
                        node.SetSolutionStepValue(VELOCITY_X,0, vc[0])
                        node.SetSolutionStepValue(VELOCITY_Y,0, 0.0)
                        node.Fix(VELOCITY_Y)
                    else:
                        vc = Vector(2)
                        vc[0]=node.GetSolutionStepValue(VELOCITY_X)
                        vc[1]=node.GetSolutionStepValue(VELOCITY_Y)
                        vc[0] = 0.0
                        vc[1] = 0.0
                        node.SetSolutionStepValue(VELOCITY_Y,0, vc[0])
                        node.SetSolutionStepValue(VELOCITY_X,0, vc[1])
                        node.Fix(VELOCITY_X)
                        node.Fix(VELOCITY_Y)



def CreateSolver(model_part, config, eul_model_part, gamma, contact_angle, zeta_dissapative_JM_x, zeta_dissapative_BM_x, zeta_dissapative_SM_x, zeta_dissapative_JM_y, zeta_dissapative_BM_y, zeta_dissapative_SM_y, surface_temp): #FOR 3D!
    fluid_solver = STMonolithicSolver(model_part, config.domain_size, eul_model_part, gamma, contact_angle,  zeta_dissapative_JM_x, zeta_dissapative_BM_x, zeta_dissapative_SM_x, zeta_dissapative_JM_y, zeta_dissapative_BM_y, zeta_dissapative_SM_y, surface_temp)

    if(hasattr(config, "alpha")):
        fluid_solver.alpha = config.alpha

    if(hasattr(config, "eul_model_part")):
        fluid_solver.eulerian_model_part = config.eul_model_part

    # definition of the convergence criteria
    if(hasattr(config, "velocity_relative_tolerance")):
        fluid_solver.rel_vel_tol = config.velocity_relative_tolerance
    if(hasattr(config, "velocity_absolute_tolerance")):
        fluid_solver.abs_vel_tol = config.velocity_absolute_tolerance
    if(hasattr(config, "pressure_relative_tolerance")):
        fluid_solver.rel_pres_tol = config.pressure_relative_tolerance
    if(hasattr(config, "pressure_absolute_tolerance")):
        fluid_solver.abs_pres_tol = config.pressure_absolute_tolerance
    if(hasattr(config, "dynamic_tau")):
        fluid_solver.dynamic_tau = config.dynamic_tau
    if(hasattr(config, "max_iteration")):
        fluid_solver.max_iter = config.max_iteration
    if(hasattr(config, "echo_level")):
        fluid_solver.echo_level = config.echo_level
    if(hasattr(config, "compute_reactions")):
        fluid_solver.compute_reactions = config.compute_reactions
    if(hasattr(config, "ReformDofSetAtEachStep")):
        fluid_solver.ReformDofSetAtEachStep = config.ReformDofSetAtEachStep
    if(hasattr(config, "divergence_cleareance_step")):
        fluid_solver.divergence_clearance_steps = config.divergence_cleareance_step

    import linear_solver_factory
    if(hasattr(config, "linear_solver_config")):
        fluid_solver.linear_solver = linear_solver_factory.ConstructSolver(
            config.linear_solver_config)

    return fluid_solver
