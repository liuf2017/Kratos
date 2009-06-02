#importing the Kratos Library
from Kratos import *
from KratosStructuralApplication import *
from KratosExternalSolversApplication import *

def AddVariables(model_part):
    model_part.AddNodalSolutionStepVariable(DISPLACEMENT);
    model_part.AddNodalSolutionStepVariable(REACTION);
    model_part.AddNodalSolutionStepVariable(BODY_FORCE);
    model_part.AddNodalSolutionStepVariable(NEGATIVE_FACE_PRESSURE);
    model_part.AddNodalSolutionStepVariable(POSITIVE_FACE_PRESSURE);
    model_part.AddNodalSolutionStepVariable(INSITU_STRESS);
    model_part.AddNodalSolutionStepVariable(FACE_LOAD);
    model_part.AddNodalSolutionStepVariable(FORCE);
    model_part.AddNodalSolutionStepVariable(DENSITY);
    model_part.AddNodalSolutionStepVariable(ROTATION);
    model_part.AddNodalSolutionStepVariable(MOMENTUM);



    print "*********************************************************************** "
    print "Variables for the Static Structural Arc Length Solution added correctly"
    print "*********************************************************************** "    


def AddDofs(model_part):
  for node in model_part.Nodes:
    #adding dofs
      node.AddDof(DISPLACEMENT_X,REACTION_X);
      node.AddDof(DISPLACEMENT_Y,REACTION_Y);
      node.AddDof(DISPLACEMENT_Z,REACTION_Z);
      #node.AddDof(ROTATION_X,MOMENTUM_X);
      #node.AddDof(ROTATION_Y,MOMENTUM_Y);
      #node.AddDof(ROTATION_Z,MOMENTUM_Z);
 
  print "*********************************************************************** "
  print "Dofs for the Static Structural Arc Length Solution added correctly"
  print "*********************************************************************** "

class StaticStructuralSolver:
    #######################################################################
    def __init__(self,model_part,domain_size):

        self.model_part    = model_part
        self.time_scheme   = ResidualBasedIncrementalUpdateStaticScheme()
        #self.time_scheme   = ParallelResidualBasedIncrementalUpdateStaticScheme()

	## Varibles de Control de Arc Lenght Method
	self.Ide                        = 20
	self.factor_delta_lmax          = 1.00
	self.toler                      = 1.0E-8
        self.norm                       = 1.0E-6 
	self.MaxIterations              = 100
       
	

        #definition of the solvers. Super_Lu Default
        #self.structure_linear_solver     =   SkylineLUFactorizationSolver()
        self.structure_linear_solver      =   SuperLUSolver()
        #pDiagPrecond = ParallelDiagonalPreconditioner()
        #self.structure_linear_solver =  ParallelCGSolver(1e-8, 5000,pDiagPrecond)
	#self.structure_linear_solver    =   Preconditioner()
        #self.structure_linear_solver    =   IterativeSolver() 

        #definition of the convergence criteria
        self.conv_criteria = DisplacementCriteria(self.norm,self.toler)
	#self.conv_criteria = ParallelDisplacementCriteria(0.000001,1e-9)

        #definition of the convergence criteria
       

        self.CalculateReactionFlag  = True
        self.ReformDofSetAtEachStep = True
        self.MoveMeshFlag           = True
	self.ApplyBodyForce         = False
        
    #######################################################################
    def Initialize(self):
        
        self.solver = 	ResidualBasedArcLenghtStrategy(self.model_part,self.time_scheme,self.structure_linear_solver,self.conv_criteria,self.Ide,self.MaxIterations,self.factor_delta_lmax, self.CalculateReactionFlag, self.ReformDofSetAtEachStep, self.MoveMeshFlag,self.ApplyBodyForce)
       
         
                 
    #######################################################################   
    def Solve(self):
        (self.solver).Solve()

    #######################################################################   
    def SetEchoLevel(self,level):
        (self.solver).SetEchoLevel(level)

      

