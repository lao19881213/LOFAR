      SUBROUTINE M7SEQ(N,INDROW,JPNTR,INDCOL,IPNTR,LIST,NGRP,MAXGRP,
     *               IWA,BWA)
      INTEGER N,MAXGRP
      INTEGER INDROW(1),JPNTR(1),INDCOL(1),IPNTR(1),LIST(N),NGRP(N),
     *        IWA(N)
      LOGICAL BWA(N)
C     **********
C
C     SUBROUTINE M7SEQ
C
C     GIVEN THE SPARSITY PATTERN OF AN M BY N MATRIX A, THIS
C     SUBROUTINE DETERMINES A CONSISTENT PARTITION OF THE
C     COLUMNS OF A BY A SEQUENTIAL ALGORITHM.
C
C     A CONSISTENT PARTITION IS DEFINED IN TERMS OF THE LOOPLESS
C     GRAPH G WITH VERTICES A(J), J = 1,2,...,N WHERE A(J) IS THE
C     J-TH COLUMN OF A AND WITH EDGE (A(I),A(J)) IF AND ONLY IF
C     COLUMNS I AND J HAVE A NON-ZERO IN THE SAME ROW POSITION.
C
C     A PARTITION OF THE COLUMNS OF A INTO GROUPS IS CONSISTENT
C     IF THE COLUMNS IN ANY GROUP ARE NOT ADJACENT IN THE GRAPH G.
C     IN GRAPH-THEORY TERMINOLOGY, A CONSISTENT PARTITION OF THE
C     COLUMNS OF A CORRESPONDS TO A COLORING OF THE GRAPH G.
C
C     THE SUBROUTINE EXAMINES THE COLUMNS IN THE ORDER SPECIFIED
C     BY THE ARRAY LIST, AND ASSIGNS THE CURRENT COLUMN TO THE
C     GROUP WITH THE SMALLEST POSSIBLE NUMBER.
C
C     NOTE THAT THE VALUE OF M IS NOT NEEDED BY M7SEQ AND IS
C     THEREFORE NOT PRESENT IN THE SUBROUTINE STATEMENT.
C
C     THE SUBROUTINE STATEMENT IS
C
C       SUBROUTINE M7SEQ(N,INDROW,JPNTR,INDCOL,IPNTR,LIST,NGRP,MAXGRP,
C                      IWA,BWA)
C
C     WHERE
C
C       N IS A POSITIVE INTEGER INPUT VARIABLE SET TO THE NUMBER
C         OF COLUMNS OF A.
C
C       INDROW IS AN INTEGER INPUT ARRAY WHICH CONTAINS THE ROW
C         INDICES FOR THE NON-ZEROES IN THE MATRIX A.
C
C       JPNTR IS AN INTEGER INPUT ARRAY OF LENGTH N + 1 WHICH
C         SPECIFIES THE LOCATIONS OF THE ROW INDICES IN INDROW.
C         THE ROW INDICES FOR COLUMN J ARE
C
C               INDROW(K), K = JPNTR(J),...,JPNTR(J+1)-1.
C
C         NOTE THAT JPNTR(N+1)-1 IS THEN THE NUMBER OF NON-ZERO
C         ELEMENTS OF THE MATRIX A.
C
C       INDCOL IS AN INTEGER INPUT ARRAY WHICH CONTAINS THE
C         COLUMN INDICES FOR THE NON-ZEROES IN THE MATRIX A.
C
C       IPNTR IS AN INTEGER INPUT ARRAY OF LENGTH M + 1 WHICH
C         SPECIFIES THE LOCATIONS OF THE COLUMN INDICES IN INDCOL.
C         THE COLUMN INDICES FOR ROW I ARE
C
C               INDCOL(K), K = IPNTR(I),...,IPNTR(I+1)-1.
C
C         NOTE THAT IPNTR(M+1)-1 IS THEN THE NUMBER OF NON-ZERO
C         ELEMENTS OF THE MATRIX A.
C
C       LIST IS AN INTEGER INPUT ARRAY OF LENGTH N WHICH SPECIFIES
C         THE ORDER TO BE USED BY THE SEQUENTIAL ALGORITHM.
C         THE J-TH COLUMN IN THIS ORDER IS LIST(J).
C
C       NGRP IS AN INTEGER OUTPUT ARRAY OF LENGTH N WHICH SPECIFIES
C         THE PARTITION OF THE COLUMNS OF A. COLUMN JCOL BELONGS
C         TO GROUP NGRP(JCOL).
C
C       MAXGRP IS AN INTEGER OUTPUT VARIABLE WHICH SPECIFIES THE
C         NUMBER OF GROUPS IN THE PARTITION OF THE COLUMNS OF A.
C
C       IWA IS AN INTEGER WORK ARRAY OF LENGTH N.
C
C       BWA IS A LOGICAL WORK ARRAY OF LENGTH N.
C
C     ARGONNE NATIONAL LABORATORY. MINPACK PROJECT. JUNE 1982.
C     THOMAS F. COLEMAN, BURTON S. GARBOW, JORGE J. MORE
C
C     **********
      INTEGER DEG,IC,IP,IPL,IPU,IR,J,JCOL,JP,JPL,JPU,L,NUMGRP
C
C     INITIALIZATION BLOCK.
C
      MAXGRP = 0
      DO 10 JP = 1, N
         NGRP(JP) = N
         BWA(JP) = .FALSE.
   10    CONTINUE
      BWA(N) = .TRUE.
C
C     BEGINNING OF ITERATION LOOP.
C
      DO 100 J = 1, N
         JCOL = LIST(J)
C
C        FIND ALL COLUMNS ADJACENT TO COLUMN JCOL.
C
         DEG = 0
C
C        DETERMINE ALL POSITIONS (IR,JCOL) WHICH CORRESPOND
C        TO NON-ZEROES IN THE MATRIX.
C
         JPL = JPNTR(JCOL)
         JPU = JPNTR(JCOL+1) - 1
         IF (JPU .LT. JPL) GO TO 50
         DO 40 JP = JPL, JPU
            IR = INDROW(JP)
C
C           FOR EACH ROW IR, DETERMINE ALL POSITIONS (IR,IC)
C           WHICH CORRESPOND TO NON-ZEROES IN THE MATRIX.
C
            IPL = IPNTR(IR)
            IPU = IPNTR(IR+1) - 1
            DO 30 IP = IPL, IPU
               IC = INDCOL(IP)
               L = NGRP(IC)
C
C              ARRAY BWA MARKS THE GROUP NUMBERS OF THE
C              COLUMNS WHICH ARE ADJACENT TO COLUMN JCOL.
C              ARRAY IWA RECORDS THE MARKED GROUP NUMBERS.
C
               IF (BWA(L)) GO TO 20
               BWA(L) = .TRUE.
               DEG = DEG + 1
               IWA(DEG) = L
   20          CONTINUE
   30          CONTINUE
   40       CONTINUE
   50    CONTINUE
C
C        ASSIGN THE SMALLEST UN-MARKED GROUP NUMBER TO JCOL.
C
         DO 60 JP = 1, N
            NUMGRP = JP
            IF (.NOT. BWA(JP)) GO TO 70
   60       CONTINUE
   70    CONTINUE
         NGRP(JCOL) = NUMGRP
         MAXGRP = MAX0(MAXGRP,NUMGRP)
C
C        UN-MARK THE GROUP NUMBERS.
C
         IF (DEG .LT. 1) GO TO 90
         DO 80 JP = 1, DEG
            L = IWA(JP)
            BWA(L) = .FALSE.
   80       CONTINUE
   90    CONTINUE
  100    CONTINUE
C
C        END OF ITERATION LOOP.
C
      RETURN
C
C     LAST CARD OF SUBROUTINE M7SEQ.
C
      END
