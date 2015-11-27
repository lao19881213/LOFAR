C$TEST DPTT
C TO RUN AS A MAIN PROGRAM REMOVE NEXT LINE
      SUBROUTINE DPTT
C***********************************************************************
C
C  EXAMPLE OF USE OF PORT PROGRAM DPOST
C
C***********************************************************************
      COMMON /CSTAK/ DS
      DOUBLE PRECISION DS(2000)
      EXTERNAL HANDLE, DPOSTD, BC, AF
      INTEGER NDX, NXH, I, K, IS(1000), NU
      INTEGER NV, NX, I1MACH
      REAL ERRPAR(2), RS(1000)
      LOGICAL LS(1000)
      COMPLEX CS(500)
      DOUBLE PRECISION DEEBSF, ERR, DABS, U(100), V(1), X(100)
      DOUBLE PRECISION DMAX1, DT, UE(100), UH(100), XH(100), WS(500)
      DOUBLE PRECISION TSTOP
      INTEGER TEMP
      EQUIVALENCE (DS(1), CS(1), WS(1), RS(1), IS(1), LS(1))
C TO ESTIMATE X AND T ERROR AS SUM.
C      U SUB T = U SUB XX + F      ON (0,1)
C WHERE F IS CHOSEN SO THAT THE SOLUTION IS
C      U(X,T) = EXP(XT).
C THE PORT LIBRARY STACK AND ITS ALIASES.
C INITIALIZE THE PORT LIBRARY STACK LENGTH.
      CALL ISTKIN(2000, 4)
      NU = 1
      NV = 0
      ERRPAR(1) = 0
      ERRPAR(2) = 1E-2
      K = 4
      NDX = 4
      TSTOP = 1
      DT = 1D-2
C CRUDE MESH.
      CALL DUMB(0D0, 1D0, NDX, K, X, NX)
C INITIAL CONDITIONS FOR U.
      CALL SETD(NX-K, 1D0, U)
      TEMP = I1MACH(2)
      WRITE (TEMP,  1) 
   1  FORMAT (36H SOLVING ON CRUDE MESH USING ERRPAR.)
      CALL DPOST(U, NU, K, X, NX, V, NV, 0D0, TSTOP, DT, AF, BC, DPOSTD,
     1   ERRPAR, HANDLE)
C GET RUN-TIME STATISTICS.
      CALL DPOSTX
C HALVE THE MESH SPACING.
      CALL DUMB(0D0, 1D0, 2*NDX-1, K, XH, NXH)
C INITIAL CONDITIONS FOR UH.
      CALL SETD(NXH-K, 1D0, UH)
      DT = 1D-2
      TEMP = I1MACH(2)
      WRITE (TEMP,  2) 
   2  FORMAT (38H SOLVING ON REFINED MESH USING ERRPAR.)
      CALL DPOST(UH, NU, K, XH, NXH, V, NV, 0D0, TSTOP, DT, AF, BC, 
     1   DPOSTD, ERRPAR, HANDLE)
C GET RUN-TIME STATISTICS.
      CALL DPOSTX
C ESTIMATE U ERROR.
      ERR = DEEBSF(K, X, NX, U, XH, NXH, UH)
      WRITE (6,  3) ERR
   3  FORMAT (24H U ERROR FROM U AND UH =, 1PE10.2)
C INITIAL CONDITIONS FOR UE.
      CALL SETD(NX-K, 1D0, UE)
      DT = 1D-2
      ERRPAR(1) = ERRPAR(1)/10.
      ERRPAR(2) = ERRPAR(2)/10.
      TEMP = I1MACH(2)
      WRITE (TEMP,  4) 
   4  FORMAT (39H SOLVING ON CRUDE MESH USING ERRPAR/10.)
      CALL DPOST(UE, NU, K, X, NX, V, NV, 0D0, TSTOP, DT, AF, BC, 
     1   DPOSTD, ERRPAR, HANDLE)
C GET RUN-TIME STATISTICS.
      CALL DPOSTX
      ERR = 0
      TEMP = NX-K
      DO  5 I = 1, TEMP
         ERR = DMAX1(ERR, DABS(U(I)-UE(I)))
   5     CONTINUE
      WRITE (6,  6) ERR
   6  FORMAT (24H U ERROR FROM U AND UE =, 1PE10.2)
      STOP 
      END
      SUBROUTINE AF(T, X, NX, U, UX, UT, UTX, NU, V, VT, NV, A, 
     1   AU, AUX, AUT, AUTX, AV, AVT, F, FU, FUX, FUT, FUTX, FV, FVT)
      INTEGER NU, NX
      INTEGER NV
      DOUBLE PRECISION T, X(NX), U(NX, NU), UX(NX, NU), UT(NX, NU), UTX(
     1   NX, NU)
      DOUBLE PRECISION V(1), VT(1), A(NX, NU), AU(NX, NU, NU), AUX(NX, 
     1   NU, NU), AUT(NX, NU, NU)
      DOUBLE PRECISION AUTX(NX, NU, NU), AV(1), AVT(1), F(NX, NU), FU(
     1   NX, NU, NU), FUX(NX, NU, NU)
      DOUBLE PRECISION FUT(NX, NU, NU), FUTX(NX, NU, NU), FV(1), FVT(1)
      INTEGER I
      DOUBLE PRECISION DEXP
      DO  1 I = 1, NX
         A(I, 1) = -UX(I, 1)
         AUX(I, 1, 1) = -1
         F(I, 1) = (X(I)-T**2)*DEXP(X(I)*T)-UT(I, 1)
         FUT(I, 1, 1) = -1
   1     CONTINUE
      RETURN
      END
      SUBROUTINE BC(T, L, R, U, UX, UT, UTX, NU, V, VT, NV, B, BU,
     1   BUX, BUT, BUTX, BV, BVT)
      INTEGER NU
      INTEGER NV
      DOUBLE PRECISION T, L, R, U(NU, 2), UX(NU, 2), UT(NU, 2)
      DOUBLE PRECISION UTX(NU, 2), V(1), VT(1), B(NU, 2), BU(NU, NU, 2),
     1   BUX(NU, NU, 2)
      DOUBLE PRECISION BUT(NU, NU, 2), BUTX(NU, NU, 2), BV(1), BVT(1)
      DOUBLE PRECISION DEXP
      B(1, 1) = U(1, 1)-1D0
      B(1, 2) = U(1, 2)-DEXP(T)
      BU(1, 1, 1) = 1
      BU(1, 1, 2) = 1
      RETURN
      END
      SUBROUTINE HANDLE(T0, U0, V0, T, U, V, NU, NXMK, NV, K, X, 
     1   NX, DT, TSTOP)
      INTEGER NXMK, NU, NX
      INTEGER NV, K
      DOUBLE PRECISION T0, U0(NXMK, NU), V0(1), T, U(NXMK, NU), V(1)
      DOUBLE PRECISION X(NX), DT, TSTOP
      COMMON /TIME/ TT
      DOUBLE PRECISION TT
      EXTERNAL UOFX
      INTEGER I1MACH
      DOUBLE PRECISION DEESFF, EU
      INTEGER TEMP
C OUTPUT AND CHECKING ROUTINE.
      IF (T0 .NE. T) GOTO 2
         TEMP = I1MACH(2)
         WRITE (TEMP,  1) T
   1     FORMAT (16H RESTART FOR T =, 1PE10.2)
         RETURN
   2  TT = T
      EU = DEESFF(K, X, NX, U, UOFX)
      TEMP = I1MACH(2)
      WRITE (TEMP,  3) T, EU
   3  FORMAT (14H ERROR IN U(X,, 1PE10.2, 4H ) =, 1PE10.2)
      RETURN
      END
      SUBROUTINE UOFX(X, NX, U, W)
      INTEGER NX
      DOUBLE PRECISION X(NX), U(NX), W(NX)
      COMMON /TIME/ T
      DOUBLE PRECISION T
      INTEGER I
      DOUBLE PRECISION DEXP
      DO  1 I = 1, NX
         U(I) = DEXP(X(I)*T)
   1     CONTINUE
      RETURN
      END
