////////////////////////////////////////////
//	Copyright (C) 2010,2011 Advanced Micro Devices, Inc. All Rights Reserved.
////////////////////////////////////////////

// clAmdFft.client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../include/clAmdFft.h"
#include "clAmdFft.client.h"
#include "clAmdFft.openCL.h"
#include "statisticalTimer.h"
#include "amd-unicode.h"

namespace po = boost::program_options;

//	This is used with the program_options class so that the user can type an integer on the command line 
//	and we store into an enum varaible
template<class _Elem, class _Traits>
std::basic_istream<_Elem, _Traits> & operator>> (std::basic_istream<_Elem, _Traits> & stream, clAmdFftLayout & layout)
{
	cl_uint tmp;
	stream >> tmp;
	layout = clAmdFftLayout(tmp);
	return stream;
}

//	Format an unsigned number with comma thousands separator
//
template<typename T>		// T could be 32-bit or 64-bit
std::basic_string<TCHAR> commatize (T number)	{
	static TCHAR scratch [8*sizeof(T)];
	register TCHAR * ptr = scratch + countOf(scratch);
	*(--ptr) = 0;
	for (int digits = 3; ; ) {
		*(--ptr) = '0' + int (number % 10);
		number /= 10;
		if (0 == number)
			break;
		if (--digits <= 0) {
			*(--ptr) = ',';
			digits = 3;
			}
	}
	return std::basic_string<TCHAR> (ptr);
}	// end of commatize ()


int _tmain( int argc, _TCHAR* argv[] )
{
	//	This helps with mixing output of both wide and narrow characters to the screen
	std::ios::sync_with_stdio( false );

	//	Define MEMORYREPORT on windows platfroms to enable debug memory heap checking
#if defined( MEMORYREPORT ) && defined( _WIN32 )
	TCHAR logPath[ MAX_PATH ];
	::GetCurrentDirectory( MAX_PATH, logPath );
	::_tcscat_s( logPath, _T( "\\MemoryReport.txt") );

	//	We leak the handle to this file, on purpose, so that the ::_CrtSetReportFile() can output it's memory 
	//	statistics on app shutdown
	HANDLE hLogFile;
	hLogFile = ::CreateFile( logPath, GENERIC_WRITE, 
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	::_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW | _CRTDBG_MODE_DEBUG );
	::_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW | _CRTDBG_MODE_DEBUG );
	::_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );

	::_CrtSetReportFile( _CRT_ASSERT, hLogFile );
	::_CrtSetReportFile( _CRT_ERROR, hLogFile );
	::_CrtSetReportFile( _CRT_WARN, hLogFile );

	int tmp = ::_CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tmp |= _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF;
	::_CrtSetDbgFlag( tmp );

	//	By looking at the memory leak report that is generated by this debug heap, there is a number with 
	//	{} brackets that indicates the incremental allocation number of that block.  If you wish to set
	//	a breakpoint on that allocation number, put it in the _CrtSetBreakAlloc() call below, and the heap
	//	will issue a bp on the request, allowing you to look at the call stack
	//	::_CrtSetBreakAlloc( 1833 );

#endif /* MEMORYREPORT */

	//	OpenCL state
	cl_context			context;
	cl_command_queue	queue;
	cl_mem				clMemBuffersIn [ 2 ] = { NULL, NULL };
	cl_mem				clMemBuffersOut[ 2 ] = { NULL, NULL };
	std::vector< cl_device_id > device_id;
	cl_event			outEvent	= NULL;
	cl_device_type		deviceType	= CL_DEVICE_TYPE_DEFAULT;
	cl_uint				deviceGpuList     = 0;	// a bitmap set

	//	FFT state
	clAmdFftPlanHandle	plHandle;
	clAmdFftResultLocation	place = CLFFT_INPLACE;
	clAmdFftLayout inLayout  = CLFFT_COMPLEX_INTERLEAVED;
	clAmdFftLayout outLayout = CLFFT_COMPLEX_INTERLEAVED;
	size_t clLengths[ 3 ];
	size_t clPadding[ 3 ] = {0, 0, 0, };  // *** TODO
	size_t clStrides[ 4 ];
	cl_uint commandQueueFlags = 0;
	size_t batchSize = 1;

	//	Local Data
	size_t buffSizeBytesIn = 0, buffSizeBytesOut = 0;
	size_t fftVectorSize= 0, fftVectorSizePadded = 0, fftBatchSize = 0;
	cl_uint nBuffersOut = 0;
	cl_uint profileCount = 0;
	clAmdFftDim	dim = CLFFT_1D;

	//	Initialize flags for FFT library
	std::auto_ptr< clAmdFftSetupData > setupData( new clAmdFftSetupData );
	OPENCL_V_THROW( clAmdFftInitSetupData( setupData.get( ) ), 
		"clAmdFftInitSetupData failed" );

	try
	{
		// Declare the supported options.
		po::options_description desc( "clFFT client command line options" );
		desc.add_options()
			( "help,h",        "produces this help message" )
			( "version,v",     "Print queryable version information from the clFFT library" )
			( "clInfo,i",      "Print queryable information of the OpenCL runtime" )
			( "gpu,g",         "Force instantiation of an OpenCL GPU device" )
			( "gpu0",          "Force instantiation of an OpenCL GPU device using gpu0" )
			( "gpu1",          "Force instantiation of an OpenCL GPU device using gpu1" )
			( "gpu2",          "Force instantiation of an OpenCL GPU device using gpu2" )
			( "gpu3",          "Force instantiation of an OpenCL GPU device using gpu3" )
			( "cpu,c",         "Force instantiation of an OpenCL CPU device" )
			( "all,a",         "Force instantiation of all OpenCL devices" )
			( "outPlace,o",    "Out of place FFT transform (default: in place)" )
			( "dumpKernels,d", "FFT engine will dump generated OpenCL FFT kernels to disk (default: dump off)" )
			( "lenX,x",        po::value< size_t >( &clLengths[ 0 ] )->default_value( 1024 ),   "Specify the length of the 1st dimension of a test array" )
			( "lenY,y",        po::value< size_t >( &clLengths[ 1 ] )->default_value( 1 ),      "Specify the length of the 2nd dimension of a test array" )
			( "lenZ,z",        po::value< size_t >( &clLengths[ 2 ] )->default_value( 1 ),      "Specify the length of the 3rd dimension of a test array" )
			( "batchSize,b",   po::value< size_t >( &batchSize )->default_value( 1 ), "If this value is greater than one, arrays will be used " )
			( "profile,p",     po::value< cl_uint >( &profileCount )->default_value( 1 ), "Time and report the kernel speed of the FFT (default: profiling off)" )
			( "inLayout",      po::value< clAmdFftLayout >( &inLayout )->default_value( CLFFT_COMPLEX_INTERLEAVED ), "Layout of input data:\n1) interleaved\n2) planar" )
			( "outLayout",     po::value< clAmdFftLayout >( &outLayout )->default_value( CLFFT_COMPLEX_INTERLEAVED ), "Layout of input data:\n1) interleaved\n2) planar" )
			;

		po::variables_map vm;
		po::store( po::parse_command_line( argc, argv, desc ), vm );
		po::notify( vm );

		if( vm.count( "version" ) )
		{
			const int indent = countOf( "clFFT client API version: " );
			tout << std::left << std::setw( indent ) << _T( "clFFT client API version: " )
				<< clAmdFftVersionMajor << _T( "." )
				<< clAmdFftVersionMinor << _T( "." )
				<< clAmdFftVersionPatch << std::endl;

			cl_uint libMajor, libMinor, libPatch;
			clAmdFftGetVersion( &libMajor, &libMinor, &libPatch );

			tout << std::left << std::setw( indent ) << _T( "clFFT runtime version: " )
				<< libMajor << _T( "." )
				<< libMinor << _T( "." )
				<< libPatch << std::endl << std::endl;
		}

		if( vm.count( "help" ) )
		{
			//	This needs to be 'cout' as program-options does not support wcout yet
			std::cout << desc << std::endl;
			return 0;
		}

		size_t mutex = ((vm.count( "gpu" ) > 0) ? 1 : 0)
			| ((vm.count( "gpu0" ) > 0) ? 1 : 0)
			| ((vm.count( "gpu1" ) > 0) ? 1 : 0)
			| ((vm.count( "gpu2" ) > 0) ? 1 : 0)
			| ((vm.count( "gpu3" ) > 0) ? 1 : 0)
			| ((vm.count( "cpu" ) > 0) ? 2 : 0)
			| ((vm.count( "all" ) > 0) ? 4 : 0);
		if ((mutex & (mutex-1)) != 0) {
			terr << _T("You have selected mutually-exclusive OpenCL device options:") << std::endl;
			if (vm.count ( "gpu" )  > 0) terr << _T("    gpu,g   Force instantiation of an OpenCL GPU device" ) << std::endl;
			if (vm.count ( "gpu0" ) > 0) terr << _T("    gpu0    Force instantiation of an OpenCL GPU device using gpu0" ) << std::endl;
			if (vm.count ( "gpu1" ) > 0) terr << _T("    gpu1    Force instantiation of an OpenCL GPU device using gpu1" ) << std::endl;
			if (vm.count ( "gpu2" ) > 0) terr << _T("    gpu2    Force instantiation of an OpenCL GPU device using gpu2" ) << std::endl;
			if (vm.count ( "gpu3" ) > 0) terr << _T("    gpu3    Force instantiation of an OpenCL GPU device using gpu3" ) << std::endl;
			if (vm.count ( "cpu" )  > 0) terr << _T("    cpu,c   Force instantiation of an OpenCL CPU device" ) << std::endl;
			if (vm.count ( "all" )  > 0) terr << _T("    all,a   Force instantiation of all OpenCL devices" ) << std::endl;
			return 1;
		}

		if( vm.count( "gpu" ) )
		{
			deviceType = CL_DEVICE_TYPE_GPU;
			deviceGpuList = ~0;
		}
		if( vm.count( "gpu0" ) )
		{
			deviceType	= CL_DEVICE_TYPE_GPU;
			deviceGpuList |= 1;
		}
		if( vm.count( "gpu1" ) )
		{
			deviceType	= CL_DEVICE_TYPE_GPU;
			deviceGpuList |= 2;
		}
		if( vm.count( "gpu2" ) )
		{
			deviceType	= CL_DEVICE_TYPE_GPU;
			deviceGpuList |= 4;
		}
		if( vm.count( "gpu3" ) )
		{
			deviceType	= CL_DEVICE_TYPE_GPU;
			deviceGpuList |= 8;
		}

		if( vm.count( "cpu" ) )
		{
			deviceType	= CL_DEVICE_TYPE_CPU;
		}

		if( vm.count( "all" ) )
		{
			deviceType	= CL_DEVICE_TYPE_ALL;
		}

		bool printInfo = false;
		if( vm.count( "clInfo" ) )
		{
			printInfo = true;
		}

		if( vm.count( "outPlace" ) )
		{
			place = CLFFT_OUTOFPLACE;
		}

		if( profileCount > 1 )
		{
			commandQueueFlags |= CL_QUEUE_PROFILING_ENABLE;
		}

		if( vm.count( "dumpKernels" ) )
		{
			setupData->debugFlags	|= CLFFT_DUMP_PROGRAMS;
		}

		//	Our command line does not specify what dimension FFT we wish to transform; we decode 
		//	this from the lengths that the user specifies for X, Y, Z.  A length of one means that
		//	The user does not want that dimension.

		for (unsigned u = 0; u < countOf(clLengths); ++u) {
			if (0 != clLengths[u])	continue;
			clLengths[u] = 1;
		}

		dim = CLFFT_1D;
		if( clLengths[ 1 ] > 1 )
		{
			dim	= CLFFT_2D;
		}
		if( clLengths[ 2 ] > 1 )
		{
			dim	= CLFFT_3D;
		}

		clStrides[ 0 ] = 1;
		clStrides[ 1 ] = clStrides[ 0 ] * (clLengths[ 0 ] + clPadding[ 0 ]);
		clStrides[ 2 ] = clStrides[ 1 ] * (clLengths[ 1 ] + clPadding[ 1 ]);
		clStrides[ 3 ] = clStrides[ 2 ] * (clLengths[ 2 ] + clPadding[ 2 ]);

		fftVectorSize	= clLengths[ 0 ] * clLengths[ 1 ] * clLengths[ 2 ];
		fftVectorSizePadded = clStrides[ 3];
		fftBatchSize	= fftVectorSizePadded * batchSize;

		switch( outLayout )
		{
		case CLFFT_COMPLEX_INTERLEAVED:
			nBuffersOut      = 1;
			buffSizeBytesOut = fftBatchSize * sizeof( std::complex< float > );
			break;
		case CLFFT_COMPLEX_PLANAR:
			nBuffersOut      = 2;
			buffSizeBytesOut = fftBatchSize * sizeof(float);
			break;
		}

		//	Fill our input buffers depending on how we want 
		switch( inLayout )
		{
		case CLFFT_COMPLEX_INTERLEAVED:
			{
				//	This call creates our openCL context and sets up our devices; expected to throw on error
				buffSizeBytesIn = fftBatchSize * sizeof( std::complex< float > );

				device_id = initializeCL( deviceType, deviceGpuList, context, printInfo );
				createOpenCLCommandQueue( context,
										  commandQueueFlags, queue,
										  device_id,
										  buffSizeBytesIn, 1, clMemBuffersIn,
										  buffSizeBytesOut, nBuffersOut, clMemBuffersOut);

				std::vector< std::complex< float > > input( fftBatchSize );

				// impulse test case
				for( cl_uint i = 0; i < fftBatchSize; ++i )
				{
					input[ i ] = 1;
				}

				OPENCL_V_THROW( clEnqueueWriteBuffer( queue, clMemBuffersIn[ 0 ], CL_TRUE, 0, buffSizeBytesIn, &input[ 0 ], 
					0, NULL, &outEvent ), 
					"clEnqueueWriteBuffer failed" );

				//for( cl_uint i = 0; i < fftBatchSize; ++i )
				//{
				//	input[ i ] = 1.23456f;
				//}

				//OPENCL_V_THROW( clEnqueueWriteBuffer( queue, clMemBuffersOut[ 0 ], CL_TRUE, 0, buffSizeBytes, &input[ 0 ], 
				//	0, NULL, &outEvent ), 
				//	"clEnqueueWriteBuffer failed" );
			}
			break;
		case CLFFT_COMPLEX_PLANAR:
			{
				//	This call creates our openCL context and sets up our devices; expected to throw on error
				buffSizeBytesIn = fftBatchSize * sizeof( float );

				device_id = initializeCL( deviceType, deviceGpuList, context, printInfo );
				createOpenCLCommandQueue( context,
										  commandQueueFlags, queue,
										  device_id,
										  buffSizeBytesIn, 2, clMemBuffersIn,
										  buffSizeBytesOut, nBuffersOut, clMemBuffersOut);

				//	Just initialize the input buffer to all 1's for now
				std::vector< float > real( fftBatchSize );
				std::vector< float > imag( fftBatchSize );
				for( cl_uint i = 0; i < fftBatchSize; ++i )
				{
					real[ i ] = 1;
					imag[ i ] = 0;
				}

				OPENCL_V_THROW( clEnqueueWriteBuffer( queue, clMemBuffersIn[ 0 ], CL_TRUE, 0, buffSizeBytesIn, &real[ 0 ], 
					0, NULL, &outEvent ), 
					"clEnqueueWriteBuffer failed" );
				OPENCL_V_THROW( clEnqueueWriteBuffer( queue, clMemBuffersIn[ 1 ], CL_TRUE, 0, buffSizeBytesIn, &imag[ 0 ], 
					0, NULL, &outEvent ), 
					"clEnqueueWriteBuffer failed" );
			}
			break;
		default:
			{
				throw std::runtime_error( "Input layout format not yet supported" );
			}
			break;
		}

	}
	catch( std::exception& e )
	{
		terr << _T( "clFFT error condition reported:" ) << std::endl << e.what() << std::endl;
		return 1;
	}

	//	Performance Data
	StatisticalTimer&	sTimer	= StatisticalTimer::getInstance( );
	sTimer.Reserve( 3, profileCount );
	sTimer.setNormalize( true );
	StatisticalTimer::sTimerID	clFFTID	= sTimer.getUniqueID( "clFFT", 0 );

	OPENCL_V_THROW( clAmdFftSetup( setupData.get( ) ), "clAmdFftSetup failed" );

	OPENCL_V_THROW( clAmdFftCreateDefaultPlan( &plHandle, context, dim, clLengths ), "clAmdFftCreateDefaultPlan failed" );

	//	Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
	OPENCL_V_THROW( clAmdFftSetResultLocation( plHandle, place ), "clAmdFftSetResultLocation failed" );
	OPENCL_V_THROW( clAmdFftSetLayout( plHandle, inLayout, outLayout ), "clAmdFftSetLayout failed" );
	OPENCL_V_THROW( clAmdFftSetPlanBatchSize( plHandle, batchSize ), "clAmdFftSetPlanBatchSize failed" );

	if ((clPadding[ 0 ] | clPadding[ 1 ] | clPadding[ 2 ]) != 0) {
		OPENCL_V_THROW (clAmdFftSetPlanInStride  ( plHandle, dim, clStrides ), "clAmdFftSetPlanInStride failed" );
		OPENCL_V_THROW (clAmdFftSetPlanOutStride ( plHandle, dim, clStrides ), "clAmdFftSetPlanOutStride failed" );
		OPENCL_V_THROW (clAmdFftSetPlanDistance  ( plHandle, clStrides[ dim ], clStrides[ dim ]), "clAmdFftSetPlanDistance failed" );
	}

	OPENCL_V_THROW( clAmdFftBakePlan( plHandle, 1, &queue, NULL, NULL ), "clAmdFftBakePlan failed" );
	
	//get the buffersize
	size_t buffersize=0;
	OPENCL_V_THROW( clAmdFftGetTmpBufSize(plHandle, &buffersize ), "clAmdFftGetTmpBufSize failed" );
		
	//allocate the intermediate buffer	
	cl_mem clMedBuffer=NULL;
		
	if (buffersize)
	{
		cl_int medstatus;
		clMedBuffer = clCreateBuffer ( context, CL_MEM_READ_WRITE, buffersize, 0, &medstatus);
		OPENCL_V_THROW( medstatus, "Creating intmediate Buffer failed" );
	}

	switch( inLayout )
	{
	case CLFFT_COMPLEX_INTERLEAVED:
	case CLFFT_COMPLEX_PLANAR:
		break;
	default:
		//	Don't recognize input layout
		return CLFFT_INVALID_ARG_VALUE;
	}

	switch( outLayout )
	{
	case CLFFT_COMPLEX_INTERLEAVED:
	case CLFFT_COMPLEX_PLANAR:
		break;
	default:
		//	Don't recognize output layout
		return CLFFT_INVALID_ARG_VALUE;
	}

	if (( place == CLFFT_INPLACE )
	&&  ( inLayout != outLayout )) {
		switch( inLayout )
		{
		case CLFFT_COMPLEX_INTERLEAVED:
			{
				assert (CLFFT_COMPLEX_PLANAR == outLayout);
				throw std::runtime_error( "Cannot use the same buffer for interleaved->planar in-place transforms" );
				break;
			}
		case CLFFT_COMPLEX_PLANAR:
			{
				assert (CLFFT_COMPLEX_INTERLEAVED == outLayout);
				throw std::runtime_error( "Cannot use the same buffer for planar->interleaved in-place transforms" );
				break;
			}
		}
	}

	//	Loop as many times as the user specifies to average out the timings
	//
	cl_mem * BuffersOut = ( place == CLFFT_INPLACE ) ? NULL :  &clMemBuffersOut[ 0 ];
	sTimer.Start(clFFTID);
	for( cl_uint i = 0; i < profileCount; ++i )
	{
		OPENCL_V_THROW( clAmdFftEnqueueTransform( plHandle, CLFFT_FORWARD, 1, &queue, 0, NULL, &outEvent, 
			&clMemBuffersIn[ 0 ], BuffersOut, clMedBuffer ),
			"clAmdFftEnqueueTransform failed" );
	}
	OPENCL_V_THROW( clFinish( queue ), "clFinish failed" );
	sTimer.Stop(clFFTID);

	if( commandQueueFlags & CL_QUEUE_PROFILING_ENABLE )
	{
		//	Remove all timings that are outside of 3 stddev; we ignore outliers to get a more consistent result
		sTimer.pruneOutliers( 3.0 );

		// windows frequency count is by seconds
		double kernelExecTimeNs	= sTimer.getAverageTime( clFFTID ) * 1e9/profileCount;
		double kernelExecGflops	= 5 * fftBatchSize * (log(static_cast<float>(fftVectorSize))/log(2.0f)) / static_cast< double >( kernelExecTimeNs );

		tout << _T( "FFT kernel execution time < ns >: " ) << commatize ((unsigned long long) kernelExecTimeNs) << std::endl;
		tout << _T( "FFT kernel execution Gflops < BatchSize*5*N*log2( N ) >: " ) << kernelExecGflops << std::endl;
	}
	sTimer.Reset( );

	// Read and check output data
	// This check is not valid if the FFT is executed multiple times inplace.
	//
	if (( place == CLFFT_OUTOFPLACE )
	||  ( profileCount == 1))
	{
		bool checkflag= false;
		switch( outLayout )
		{
		case CLFFT_COMPLEX_INTERLEAVED:
			{
				std::vector< std::complex< float > > output( fftBatchSize );

				if( place == CLFFT_INPLACE )
				{
					OPENCL_V_THROW( clEnqueueReadBuffer( queue, clMemBuffersIn[ 0 ], CL_TRUE, 0, buffSizeBytesIn, &output[ 0 ], 
						0, NULL, NULL ),
						"Reading the result buffer failed" );
				}
				else
				{
					OPENCL_V_THROW( clEnqueueReadBuffer( queue, clMemBuffersOut[ 0 ], CL_TRUE, 0, buffSizeBytesOut, &output[ 0 ], 
						0, NULL, NULL ),
						"Reading the result buffer failed" );
				}

				//check output data
				for( cl_uint i = 0; i < fftBatchSize; ++i )
				{
					if (0 == (i % fftVectorSizePadded))
					{
						if (output[i].real() != fftVectorSize) 
						{
							checkflag = true;
							break;
						}
					}
					else
					{
						if (output[ i ].real() != 0)
						{
							checkflag = true;
							break;
						}
					}

					if (output[ i ].imag() != 0)
					{
						checkflag = true;
						break;
					}
				}
			}
			break;
		case CLFFT_COMPLEX_PLANAR:
			{
				std::valarray< float > real( fftBatchSize );
				std::valarray< float > imag( fftBatchSize );

				if( place == CLFFT_INPLACE )
				{
					OPENCL_V_THROW( clEnqueueReadBuffer( queue, clMemBuffersIn[ 0 ], CL_TRUE, 0, buffSizeBytesIn, &real[ 0 ], 
						0, NULL, NULL ),
						"Reading the result buffer failed" );
					OPENCL_V_THROW( clEnqueueReadBuffer( queue, clMemBuffersIn[ 1 ], CL_TRUE, 0, buffSizeBytesIn, &imag[ 0 ], 
						0, NULL, NULL ),
						"Reading the result buffer failed" );
				}
				else
				{
					OPENCL_V_THROW( clEnqueueReadBuffer( queue, clMemBuffersOut[ 0 ], CL_TRUE, 0, buffSizeBytesOut, &real[ 0 ], 
						0, NULL, NULL ),
						"Reading the result buffer failed" );
					OPENCL_V_THROW( clEnqueueReadBuffer( queue, clMemBuffersOut[ 1 ], CL_TRUE, 0, buffSizeBytesOut, &imag[ 0 ], 
						0, NULL, NULL ),
						"Reading the result buffer failed" );
				}

				//  Check output data
				//  The output data might not be contiguous in the output buffer, if there
				//  is any padding in any dimension, so we need to access slices of the buffer.
				//  We treat the data buffers as arrays of 3D arrays in all cases.
				//  If this is a 1D test, then
				//     clLength[ 1 ] and clLength[ 2] will be 1.
				//     The first element of every 1D slice will be nonzero.
				//  If this is a 2D test, then
				//     clLength[ 2 ] will be a.
				//     The first elment of every 2D slice will be nonzero.
				//  If this is a 3D test, then
				//     The first element of every 3D slice will be nonzero.
				//
				for (unsigned ub = 0; ub < batchSize; ++ub) {
					std::slice slice3D (ub * clStrides[ 3 ], clStrides[ 3 ], 1);
					std::valarray<float> real3D (real[ slice3D ]);
					for (unsigned uz = 0; uz < clLengths[2]; ++uz) {
						std::slice slice2D (uz * clStrides[ 2 ], clStrides[ 2 ], 1);
						std::valarray<float> real2D (real[ slice2D ]);
						bool nzZ = (dim == CLFFT_3D) && (0 == uz);
						for (unsigned uy = 0; uy < clLengths[1]; ++uy) {
							std::slice slice1D (uy * clStrides[ 1], clStrides[ 1], 1);
							std::valarray<float> real1D (real2D [ slice1D ]);
							bool nzY = (nzZ || (dim == CLFFT_2D)) && (0 == uy);
							for (unsigned ux = 0; ux < clLengths[0]; ++ux) {
								bool nzX = (nzY || (dim == CLFFT_1D)) && (0 == ux);
								float expected = nzX ? float (fftVectorSize) : 0.0f;
								if (real1D[ux] != expected)
									checkflag = true;
							}
						}
					}
				}

				////check output data
				//for( cl_uint i = 0; i < fftBatchSize; ++i )
				//{
				//	if (0 == (i % fftVectorSizePadded))
				//	{
				//		if (real[i] != fftVectorSize) 
				//		{
				//			checkflag = true;
				//			break;
				//		}
				//	}
				//	else
				//	{
				//		if (real[i] != 0)
				//		{
				//			checkflag = true;
				//			break;
				//		}
				//	}

				//	if (imag[i] != 0)
				//	{
				//		checkflag = true;
				//		break;
				//	}
				//}
			}
			break;
		default:
			{
				throw std::runtime_error( "Input layout format not yet supported" );
			}
			break;
		}
	
		if (checkflag) 
		{
			std::cout << "\n\n\t\tClient Test *****FAIL*****" << std::endl;
		}
		else
		{
			std::cout << "\n\n\t\tClient Test *****PASS*****" << std::endl;
		}
	}

	OPENCL_V_THROW( clAmdFftDestroyPlan( &plHandle ), "clAmdFftDestroyPlan failed" );
	OPENCL_V_THROW( clAmdFftTeardown( ), "clAmdFftTeardown failed" );

	cleanupCL( &context, &queue, countOf( clMemBuffersIn ), clMemBuffersIn, countOf( clMemBuffersOut ), clMemBuffersOut, &outEvent );

	return 0;
}
