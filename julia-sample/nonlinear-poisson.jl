using Pkg
using ArgParse
using SparseArrays
using LinearAlgebra





argdef = ArgParseSettings()
add_arg_table(argdef,
    "--vispy", Dict(:help => "call python visualization",
#                    :default => false,
                    :action => :store_true)

)

args = parse_args(argdef)




const dirichlet_penalty=1.0e30
const n=11
const xl=0.0
const xr=1.0
const eps=1.0
const ul=1.0
const ur=0.0






function eval_and_assemble(n,mat,V,X,U,F,eps, ul,ur, f, df)
    for i=1:n-1
        K=i
        L=i+1
        mat[K,L]=0.0
        mat[L,K]=0.0
        mat[K,K]=0.0
        mat[L,L]=0.0
    end
    for i=1:n
        F[i]=V[i]*f(U[i])
        mat[i,i]+=V[i]*df(U[i])
    end
    F[1]+=dirichlet_penalty*(U[1]-ul)
    F[n]+=dirichlet_penalty*(U[n]-ur)

    for i=1:n-1
        K=i
        L=i+1
        h=X[L]-X[K]
        deps=eps/h
        F[K]+=deps*(U[K]-U[L])
        F[L]-=deps*(U[K]-U[L])
        mat[K,L]-=eps/h
        mat[L,K]-=eps/h
        mat[K,K]+=eps/h
        mat[L,L]+=eps/h
    end
    mat[1,1]+=dirichlet_penalty
    mat[n,n]+=dirichlet_penalty

end

function solve(n,xl,xr, eps, ul,ur, f, df)
    mat=SparseArrays.spzeros(n,n)
    # initialize matrix pattern
    V=zeros(n)
    X=zeros(n)
    U=zeros(n)
    F=zeros(n)
    X[1]=xl
    h=(xr-xl)/(n-1)
    for i=2:n
        X[i]=X[i-1]+h
    end
    for i=1:n-1
        K=i
        L=i+1
        h=X[L]-X[K]
        V[K]+=0.5*h
        V[L]+=0.5*h
    end
    oldnorm=1.0
    for ii=1:100
        eval_and_assemble(n,mat,V,X,U,F,eps, ul,ur, f, df)
        lufact=LinearAlgebra.lu(mat)
        DU=lufact\F
        U=U-DU
        norm=0.0
        for i=1:n
            norm+=DU[i]*DU[i]
        end
        norm=sqrt(norm)
        println("it=",ii," norm=",norm, " cont=", norm/oldnorm)
        if norm<1.0e-10
            break
        end
        oldnorm=norm
    end
    return X,U
end



function f(x)
    return 100.0*x*x
end

function df(x)
    return 100.0*2.0*x
end


(X,U)=solve(n,xl,xr, eps, ul,ur,f,df)


if args["vispy"]
    using PyPlot
    plot(X,U)
    pause(1.0e-10)
    waitforbuttonpress()
end
