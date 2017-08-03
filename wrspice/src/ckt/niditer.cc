
/*========================================================================*
 *                                                                        *
 *  Whiteley Research Inc, Sunnyvale, CA USA, http://wrcad.com            *
 *  Copyright (C) 2017 Whiteley Research Inc, all rights reserved.        *
 *                                                                        *
 *  Licensed under the Apache License, Version 2.0 (the "License");       *
 *  you may not use this file except in compliance with the License.      *
 *  You may obtain a copy of the License at                               *
 *                                                                        *
 *        http://www.apache.org/licenses/LICENSE-2.0                      *
 *                                                                        *
 *  See the License for the specific language governing permissions       *
 *  and limitations under the License.                                    *
 *                                                                        *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      *
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES      *
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-        *
 *   INFRINGEMENT.  IN NO EVENT SHALL WHITELEY RESEARCH INCORPORATED      *
 *   OR STEPHEN R. WHITELEY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER     *
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,      *
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE       *
 *   USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                        *
 *========================================================================*
 *               XicTools Integrated Circuit Design System                *
 *                                                                        *
 * WRspice Circuit Simulation and Analysis Tool                           *
 *                                                                        *
 *========================================================================*
 $Id:$
 *========================================================================*/

/***************************************************************************
JSPICE3 adaptation of Spice3f2 - Copyright (c) Stephen R. Whiteley 1992
Copyright 1990 Regents of the University of California.  All rights reserved.
Authors: 1988 Jaijeet S. Roychowdhury
         1993 Stephen R. Whiteley
****************************************************************************/

#include "circuit.h"
#include "spmatrix.h"


// This is absolutely the same as NIacIter, except that the RHS
// vector is stored before acLoad so that it is not lost. Moreover,
// acLoading is done only if reordering is necessary
//
int
sCKT::NIdIter()
{
    CKTmatrix->spSetComplex();
    if (CKTmatrix->spDataAddressChange()) {
        int error = resetup();
        if (error)
            return (error);
    }

    for (;;) {
        CKTnoncon = 0;

        if (CKTniState & NIACSHOULDREORDER) {
            int error = CKTmatrix->spOrderAndFactor(0,
                CKTcurTask->TSKpivotRelTol, CKTcurTask->TSKpivotAbsTol, 1);
            CKTniState &= ~NIACSHOULDREORDER;
            if (error != 0) {
                // either singular equations or no memory, in either case,
                // let caller handle problem
                //
                return (error);
            }
        }
        else {
            int error = CKTmatrix->spFactor();
            if (error != 0) {
                if (error == E_SINGULAR) {
                    // the problem is that the matrix can't be solved with
                    // the current LU factorization.  Maybe if we reload and
                    // try to reorder again it will help...
                    //
                    CKTniState |= NIACSHOULDREORDER;
                    double *tmp = CKTrhs;
                    CKTrhs = CKTrhsSpare;
                    CKTrhsSpare = tmp;

                    tmp = CKTirhs;
                    CKTirhs = CKTirhsSpare;
                    CKTirhsSpare = tmp;

                    error = acLoad();
                    if (error) return(error);
    
                    tmp = CKTrhs;
                    CKTrhs = CKTrhsSpare;
                    CKTrhsSpare = tmp;

                    tmp = CKTirhs;
                    CKTirhs = CKTirhsSpare;
                    CKTirhsSpare = tmp;
                    continue;
                }
                return (error); // can't handle E_BADMATRIX, so let caller
            }
        }
        break;
    }
    CKTmatrix->spSolve(CKTrhs, CKTrhs, CKTirhs, CKTirhs);

    *CKTrhs = 0;
    *CKTrhsSpare = 0;
    *CKTrhsOld = 0;
    *CKTirhs = 0;
    *CKTirhsSpare = 0;
    *CKTirhsOld = 0;

    double *tmp = CKTirhsOld;
    CKTirhsOld = CKTirhs;
    CKTirhs = tmp;

    tmp = CKTrhsOld;
    CKTrhsOld = CKTrhs;
    CKTrhs = tmp;
    return (OK);
}
