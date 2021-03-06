.data
.align 16
savedFloats: .zero 512

.text
.global       rainPosUpdateSSE
.global       rainPosUpdateAVX
.global   particlePosUpdateSSE
.global   particlePosUpdateAVX
.global  sparticlePosUpdateSSE
.global  sparticlePosUpdateAVX

.ifdef USE_GOT

.macro movVarl sym reg
  movq \sym@GOTPCREL(%rip), %rdx
  movl (%rdx), \reg
.endm

.macro movVarq sym reg
  movq \sym@GOTPCREL(%rip), %rdx
  leaq (%rdx), \reg
.endm

.else

.macro movVarl sym reg
  movl \sym(%rip), \reg
.endm

.macro movVarq sym reg
  leaq \sym(%rip), \reg
.endm

.endif

particlePosUpdateSSE:
  fxsave savedFloats(%rip)
  movVarl particleCount, %ecx
  shrl $2,%ecx
  inc %ecx

  movVarq   particles, %rax
  movVarq glParticles, %rdx

.particleUpdateLoopSSE:
  movaps   (%rdx),%xmm0
  movaps   (%rax),%xmm1
  movaps 16(%rdx),%xmm2
  movaps 16(%rax),%xmm3
  movaps 32(%rdx),%xmm4
  movaps 32(%rax),%xmm5
  movaps 48(%rdx),%xmm6
  movaps 48(%rax),%xmm7

  addps  %xmm1,%xmm0
  addps  %xmm3,%xmm2
  addps  %xmm5,%xmm4
  addps  %xmm7,%xmm6

  movaps %xmm0,  (%rdx)
  movaps %xmm2,16(%rdx)
  movaps %xmm4,32(%rdx)
  movaps %xmm6,48(%rdx)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .particleUpdateLoopSSE
  fxrstor savedFloats(%rip)
  ret

particlePosUpdateAVX:
  fxsave savedFloats(%rip)
  movVarl particleCount, %ecx
  shrl $2,%ecx
  inc %ecx

  movVarq   particles, %rax
  movVarq glParticles, %rdx

.particleUpdateLoopAVX:
  vmovaps   (%rdx),%ymm0
  vmovaps   (%rax),%ymm1
  vmovaps 32(%rdx),%ymm2
  vmovaps 32(%rax),%ymm3

  vaddps  %ymm1,%ymm0,%ymm4
  vaddps  %ymm3,%ymm2,%ymm5

  vmovaps %ymm4,  (%rdx)
  vmovaps %ymm5,32(%rdx)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .particleUpdateLoopAVX
  fxrstor savedFloats(%rip)
  ret

sparticlePosUpdateSSE:
  fxsave savedFloats(%rip)
  movVarl sparticleCount, %ecx
  shrl $2,%ecx
  inc %ecx

  movVarq sparticleVV, %rdx
  movaps   (%rdx),%xmm4
  movaps 16(%rdx),%xmm5
  movaps 32(%rdx),%xmm6
  movaps 48(%rdx),%xmm7

  movVarq   sparticles, %rax
  movVarq glSparticles, %rdx

.sparticleUpdateLoopSSE:
  movaps   (%rdx),%xmm0
  movaps   (%rax),%xmm1
  movaps 16(%rdx),%xmm2
  movaps 16(%rax),%xmm3

  addps  %xmm1,%xmm0
  addps  %xmm4,%xmm1
  addps  %xmm3,%xmm2
  addps  %xmm5,%xmm3

  movaps %xmm0,  (%rdx)
  movaps %xmm1,  (%rax)
  movaps %xmm2,16(%rdx)
  movaps %xmm3,16(%rax)

  movaps 32(%rdx),%xmm0
  movaps 32(%rax),%xmm1
  movaps 48(%rdx),%xmm2
  movaps 48(%rax),%xmm3

  addps  %xmm1,%xmm0
  addps  %xmm6,%xmm1
  addps  %xmm3,%xmm2
  addps  %xmm7,%xmm3

  movaps %xmm0,32(%rdx)
  movaps %xmm1,32(%rax)
  movaps %xmm2,48(%rdx)
  movaps %xmm3,48(%rax)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .sparticleUpdateLoopSSE
  fxrstor savedFloats(%rip)
  ret

sparticlePosUpdateAVX:
  fxsave savedFloats(%rip)
  movVarl sparticleCount, %ecx 
  shrl $2,%ecx
  inc %ecx

  movVarq sparticleVV, %rdx
  vmovaps   (%rdx),%ymm4
  vmovaps 32(%rdx),%ymm5

  movVarq   sparticles, %rax
  movVarq glSparticles, %rdx

.sparticleUpdateLoopAVX:
  vmovaps   (%rdx),%ymm0
  vmovaps   (%rax),%ymm1
  vmovaps 32(%rdx),%ymm2
  vmovaps 32(%rax),%ymm3

  vaddps  %ymm1,%ymm0,%ymm6
  vaddps  %ymm4,%ymm1,%ymm7
  vaddps  %ymm3,%ymm2,%ymm8
  vaddps  %ymm5,%ymm3,%ymm9

  vmovaps %ymm6,  (%rdx)
  vmovaps %ymm7,  (%rax)
  vmovaps %ymm8,32(%rdx)
  vmovaps %ymm9,32(%rax)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .sparticleUpdateLoopAVX
  fxrstor savedFloats(%rip)
  ret

rainPosUpdateSSE:
  fxsave savedFloats(%rip)
  movVarl rainCount, %ecx
  shrl $1,%ecx
  inc %ecx

  movVarq rainVel, %rdx
  movaps (%rdx),%xmm4


  movVarq rainDrops, %rax
  movVarq glRainDrops, %rdx

.rainPosUpdateLoopSSE:
  movaps   (%rdx),%xmm0
  movaps   (%rax),%xmm1
  movaps 16(%rdx),%xmm2
  movaps 16(%rax),%xmm3

  addps  %xmm1,%xmm0
  addps  %xmm4,%xmm1
  addps  %xmm3,%xmm2
  addps  %xmm4,%xmm3

  movaps %xmm0,  (%rdx)
  movaps %xmm1,  (%rax)
  movaps %xmm2,16(%rdx)
  movaps %xmm3,16(%rax)

  add $32,%rdx
  add $32,%rax
  dec %ecx
  jnz .rainPosUpdateLoopSSE
.rainPosUpdateEndSSE:
  fxrstor savedFloats(%rip)
  ret

rainPosUpdateAVX:
  fxsave savedFloats(%rip)
  movVarl rainCount, %ecx
  shrl $2,%ecx
  inc %ecx

  movVarq rainVel, %rdx
  vmovaps (%rdx),%ymm8

  movVarq   rainDrops, %rax
  movVarq glRainDrops, %rdx

.rainPosUpdateLoopAVX:
  vmovaps   (%rdx),%ymm0
  vmovaps   (%rax),%ymm1
  vmovaps 32(%rdx),%ymm2
  vmovaps 32(%rax),%ymm3

  vaddps  %ymm1,%ymm0,%ymm0
  vaddps  %ymm8,%ymm1,%ymm1
  vaddps  %ymm3,%ymm2,%ymm2
  vaddps  %ymm8,%ymm3,%ymm3

  vmovaps %ymm0,  (%rdx)
  vmovaps %ymm1,  (%rax)
  vmovaps %ymm2,32(%rdx)
  vmovaps %ymm3,32(%rax)

  add $64,%rdx
  add $64,%rax
  dec %ecx
  jnz .rainPosUpdateLoopAVX
  fxrstor savedFloats(%rip)
  ret
