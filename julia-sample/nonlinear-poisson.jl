#
# Generic part, perhaps this would be a package 
#

# Autodiff magic, get it via Pkg.add
using ForwardDiff, DiffResults

# These are in the standard distro
using SparseArrays
using LinearAlgebra


#
# Structure containing "physics" information
#
struct Physics
  eps::Float64 # dielectric constant
  dirichlet_values::Array{Float64,1} # boundary values
  nonlinearity  # nonlinearity function (u- dependent)
  source  # source term ("doping", x- dependent)
end



# Structure for discrete geometry (for finite volume method)
# This can be easily generalized to higher dimensions 
# and unstructured grids
struct DiscreteGeometry
    SpaceDim::Int32
    GridDim::Int32
    Points::Array{Float64,2}
    Edges::Array{Int32,2}
    EdgeFactors::Array{Float64,1}
    NodeFactors::Array{Float64,1}
    BPoints::Array{Int32,1}

    # Constructor for 1D discrete geometry
    function DiscreteGeometry(n,xl,xr)
        pts=reshape(collect(range(xl,stop=xr,length=n)),n,1)
        eds=zeros(Int32,n-1,2)
        efac=zeros(n-1)
        nfac=zeros(n)
        for i=1:n-1
            eds[i,1]=i
            eds[i,2]=i+1
            h=pts[i+1]-pts[i]
            efac[i]=1.0/h
            nfac[i]+=0.5*h
            nfac[i+1]+=0.5*h
        end
        bpoints=zeros(Int32,2)
        bpoints[1]=1
        bpoints[2]=n
        new(1,n,pts,eds,efac,nfac,bpoints)
    end

end

mutable struct NewtonControl
     tolerance::Float64
     damp::Float64
     maxiter::Int32
     function NewtonControl()
         new(1.0e-10,1.0,100)
     end
end


const dirichlet_penalty=1.0e30

# Input: geometry, physics
# Output: Array of solution values

function solve(geom,physics;control=NewtonControl())

    #
    # Nonlinear operator evaluation + Jacobian assembly
    #
    function eval_and_assemble(geom,physics,M,U,F)
        nnodes=geom.GridDim
        nedges=size(geom.Edges,1)

        # Reset matrix
        for i=1:nedges
            K=geom.Edges[i,1]
            L=geom.Edges[i,2]
            M[K,L]=0.0
            M[L,K]=0.0
            M[K,K]=0.0
            M[L,L]=0.0
        end
        
        # Assemble nonlinear term + source using autodifferencing via ForwardDiff
        result=DiffResults.DiffResult(U[1],U[1])
        for i=1:nnodes
            result=ForwardDiff.derivative!(result,physics.nonlinearity,U[i])
            F[i]=geom.NodeFactors[i]*DiffResults.value(result)-physics.source(geom.Points[i])
            M[i,i]+=geom.NodeFactors[i]*DiffResults.derivative(result)
        end
        
        # Assemble main part
        for i=1:nedges
            K=geom.Edges[i,1]
            L=geom.Edges[i,2]
            deps=eps*geom.EdgeFactors[i]
            F[K]+=deps*(U[K]-U[L])
            F[L]-=deps*(U[K]-U[L])
            M[K,L]-=deps
            M[L,K]-=deps
            M[K,K]+=deps
            M[L,L]+=deps
        end
        
        # Assemble boundary value part using Dirichlet penalty method
        nbc=length(geom.BPoints)
        for i=1:nbc
            ibc=geom.BPoints[i]
            F[ibc]+=dirichlet_penalty*(U[ibc]-physics.dirichlet_values[i])
            M[ibc,ibc]+=dirichlet_penalty
        end
    end
    

    # Set up solution data
    n=geom.GridDim
    M=SparseArrays.spzeros(n,n) # Jacobi matrix
    U=zeros(n) # Solution vector
    F=zeros(n) # Vector holding residual
    converged=false

    # Newton iteration (quick and dirty...)
    oldnorm=1.0
    for ii=1:control.maxiter
        eval_and_assemble(geom,physics,M,U,F)
        
        # Sparse LU factorization
        # Here, we miss the possibility to re-use the 
        # previous symbolic information
        lufact=LinearAlgebra.lu(M)
        
        # LU triangular solve gives Newton update
        DU=lufact\F

        U=U-control.damp*DU

        # Check for norm of update, if small finish
        norm=0.0
        for i=1:n
            norm+=DU[i]*DU[i]
        end
        norm=sqrt(norm)
        println("it=",ii," norm=",norm, " cont=", norm/oldnorm)
        if norm<control.tolerance
            converged=true
            break
        end

        oldnorm=norm
    end
    if !converged
        println("error: no convergence")
        exit(1)
    end
    return U
end





#########################################
# specific part

# Command line argument parser, get it via Pkg.add
using ArgParse

argdef = ArgParseSettings()
add_arg_table(argdef,
    "--vispy", Dict(:help => "call python visualization",
#                    :default => false,
                    :action => :store_true),
    "--n", Dict(:help => "problem size",
                :arg_type => Int,
                :default => 11)

)

args = parse_args(argdef)




const n=args["n"]
const xl=0.0
const xr=1.0
const eps=1.0
const dirichlet_values=[1.0,0.0]

function nonlinearity(u)
    return 100.0*u*u+exp(0.1*u)
end

function source(x)
    return 10*x
end





geom=DiscreteGeometry(n,xl,xr)


physics=Physics(eps,dirichlet_values,nonlinearity,source)



U=solve(geom,physics)


# plotting
if args["vispy"]
    using PyPlot
    plot(geom.Points,U)
    pause(1.0e-10)
    waitforbuttonpress()
end
