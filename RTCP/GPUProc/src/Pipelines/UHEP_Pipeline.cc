#include "lofar_config.h"    
#define __CL_ENABLE_EXCEPTIONS

#include "Common/LofarLogger.h"
#include "global_defines.h"
#include "Interface/Parset.h"
#include "OpenMP_Support.h"
#include <iostream>

#include "WorkQueues/UHEP_WorkQueue.h"
#include "UHEP_Pipeline.h"

namespace LOFAR
{
    namespace RTCP 
    {
        UHEP_Pipeline::UHEP_Pipeline(const Parset &ps)
            :
        Pipeline(ps),
            beamFormerCounter("beamformer"),
            transposeCounter("transpose"),
            invFFTcounter("inv. FFT"),
            invFIRfilterCounter("inv. FIR"),
            triggerCounter("trigger"),
            beamFormerWeightsCounter("BF weights"),
            samplesCounter("samples")
        {
            double startTime = omp_get_wtime();

#pragma omp parallel sections
            {
#pragma omp section
                beamFormerProgram = createProgram("UHEP/BeamFormer.cl");
#pragma omp section
                transposeProgram = createProgram("UHEP/Transpose.cl");
#pragma omp section
                invFFTprogram = createProgram("UHEP/InvFFT.cl");
#pragma omp section
                invFIRfilterProgram = createProgram("UHEP/InvFIR.cl");
#pragma omp section
                triggerProgram = createProgram("UHEP/Trigger.cl");
            }

            std::cout << "compile time = " << omp_get_wtime() - startTime << std::endl;
        }

        void UHEP_Pipeline::doWork()
        {
            float delaysAtBegin[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS] __attribute__((aligned(32)));
            float delaysAfterEnd[ps.nrBeams()][ps.nrStations()][NR_POLARIZATIONS] __attribute__((aligned(32)));
            float phaseOffsets[ps.nrStations()][NR_POLARIZATIONS] __attribute__((aligned(32)));

            memset(delaysAtBegin, 0, sizeof delaysAtBegin);
            memset(delaysAfterEnd, 0, sizeof delaysAfterEnd);
            memset(phaseOffsets, 0, sizeof phaseOffsets);
            delaysAtBegin[0][2][0] = 1e-6, delaysAfterEnd[0][2][0] = 1.1e-6;

#pragma omp parallel num_threads((profiling ? 1 : 2) * nrGPUs)
            try
            {
                UHEP_WorkQueue(*this, omp_get_thread_num()).doWork(&delaysAtBegin[0][0][0], &delaysAfterEnd[0][0][0], &phaseOffsets[0][0]);
            } catch (cl::Error &error) {
#pragma omp critical (cerr)
                std::cerr << "OpenCL error: " << error.what() << ": " << errorMessage(error.err()) << std::endl;
                exit(1);
            }
        }

        


    }
}

