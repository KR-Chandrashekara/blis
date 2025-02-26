##Copyright (C) 2023 - 2024, Advanced Micro Devices, Inc. All rights reserved.##

# Import modules
import os
import sys

def main():
	# Obtain the script name.
	path, script_name = os.path.split(sys.argv[0])
	print( " " )
	print( " %s" % script_name )
	print( " " )
	print( " Configure BLIS's CMake system for compilation using a specified" )
	print( " configuration directory." )
	print( " " )
	print( " Usage:" )
	print( " " )
	print( "   cmake ..  [Options] -DBLIS_CONFIG_FAMILY=confname" )
	print( " " )
	print(" Arguments:")
	print(" ")
	print("   confname      The name of the sub-directory inside of the 'config'")
	print("                 directory containing the desired BLIS configuration.")
	print("                 Currently, only amdzen, zen, zen2, zen3, zen4, zen5 and generic")
	print("                 configuration options are supported.")
	print("                 Note that confname MUST be specified; if it is not,")
	print("                 configure will complain. To build a completely generic")
	print("                 implementation, use the 'generic' configuration.")
	print(" ")
	print( " Options:" )
	print( " " )
	print( "   -DCMAKE_INSTALL_PREFIX=PREFIX" )
	print( " " )
	print( "                 The common installation prefix for all files." )
	print( "                 If this option is not given, PREFIX defaults to '/usr/local/'." )
	print( "                 on UNIX and c:/Program Files/${PROJECT_NAME} on Windows." )
	print( " " )
	print( "   -DENABLE_DEBUG=DEBUG" )
	print( " " )
	print( "                 Enable debugging symbols in the library." )
	print( "                 DEBUG is 'off' by default. If argument" )
	print( "                 DEBUG is given as 'opt', then optimization flags are" )
	print( "                 kept in the framework, otherwise optimization is" )
	print( "                 turned off. Available options are 'opt', 'noopt' and 'off'." )
	print( " " )
	print( "   -DBUILD_SHARED_LIBS=ON or -DBUILD_SHARED_LIBS=OFF" )
	print( " " )
	print( "                 Enable building the shared BLIS library (default)." )
	print( "                 If the shared library build is disabled, the static library" )
	print( "                 is built." )
	print( " " )
	print( "   -DBUILD_STATIC_LIBS=ON or -DBUILD_STATIC_LIBS=OFF" )
	print( " " )
	print( "                 Enable building the static BLIS library (default) (Linux only)." )
	print( "                 On Linux, we can have builds for both shared and static libraries." )
	print( " " )
	print( "   -DEXPORT_SHARED=[SYMBOLS]" )
	print( " " )
	print( "                 Specify the subset of library symbols that are exported" )
	print( "                 within a shared library. Valid values for SYMBOLS are:" )
	print( "                 'public' (the default) and 'all'. By default, only" )
	print( "                 functions and variables that belong to public APIs are" )
	print( "                 exported in shared libraries. However, the user may" )
	print( "                 instead export all symbols in BLIS, even those that were" )
	print( "                 intended for internal use only. Note that the public APIs" )
	print( "                 encompass all functions that almost any user would ever" )
	print( "                 want to call, including the BLAS/CBLAS compatibility APIs" )
	print( "                 as well as the basic and expert interfaces to the typed" )
	print( "                 and object APIs that are unique to BLIS. Also note that" )
	print( "                 changing this option to 'all' will have no effect in some" )
	print( "                 environments, such as when compiling with clang on" )
	print( "                 Windows." )
	print( " " )
	print( "   -DENABLE_THREADING=MODEL" )
	print( " " )
	print( "                 Enable threading in the library, using threading model" )
	print( "                 MODEL={openmp, pthreads, no}. If MODEL=no threading will be" )
	print( "                 disabled. The default is 'no'." )
	print( " " )
	print( "   -DENABLE_SYSTEM=ON or -DENABLE_SYSTEM=OFF")
	print( " " )
	print( "                 Enable conventional operating system support, such as" )
	print( "                 pthreads for thread-safety. The default state is enabled." )
	print( "                 However, in rare circumstances you may wish to configure" )
	print( "                 BLIS for use with a minimal or nonexistent operating" )
	print( "                 system (e.g. hardware simulators). In these situations," )
	print( "                 -DENABLE_SYSTEM=OFF may be used to jettison all compile-time" )
	print( "                 and link-time dependencies outside of the standard C" )
	print( "                 library. When disabled, this option also forces the use" )
	print( "                 of -DENABLE_THREADING=no." )
	print( " " )
	print( "   -DENABLE_PBA_POOLS=ON or -DENABLE_PBA_POOLS=OFF" )
	print( "   -DENABLE_SBA_POOLS=ON or -DENABLE_SBA_POOLS=OFF" )
	print( " " )
	print( "                 Disable (enabled by default) use of internal memory pools" )
	print( "                 within the packing block allocator (pba) and/or the small" )
	print( "                 block allocator (sba). The former is used to allocate" )
	print( "                 memory used to pack submatrices while the latter is used" )
	print( "                 to allocate control/thread tree nodes and thread" )
	print( "                 communicators. Both allocations take place in the context" )
	print( "                 of level-3 operations. When the pba is disabled, the" )
	print( "                 malloc()-like function specified by BLIS_MALLOC_POOL is" )
	print( "                 called on-demand whenever a packing block is needed, and" )
	print( "                 when the sba is disabled, the malloc()-like function" )
	print( "                 specified by BLIS_MALLOC_INTL is called whenever a small" )
	print( "                 block is needed, with the two allocators calling free()-" )
	print( "                 like functions BLIS_FREE_POOL and BLIS_FREE_INTL," )
	print( "                 respectively when blocks are released. When enabled," )
	print( "                 either or both pools are populated via the same functions" )
	print( "                 mentioned previously, and henceforth blocks are checked" )
	print( "                 out and in. The library quickly reaches a state in which" )
	print( "                 it no longer needs to call malloc() or free(), even" )
	print( "                 across many separate level-3 operation invocations." )
	print( " " )
	print( "   -DENABLE_MEM_TRACING=ON or -DENABLE_MEM_TRACING=OFF" )
	print( " " )
	print( "                 Enable (disable by default) output to stdout that traces" )
	print( "                 the allocation and freeing of memory, including the names" )
	print( "                 of the functions that triggered the allocation/freeing." )
	print( "                 Enabling this option WILL NEGATIVELY IMPACT PERFORMANCE." )
	print( "                 Please use only for informational/debugging purposes." )
	print( " " )
	print( "   -DINT_SIZE=SIZE" )
	print( " " )
	print( "                 Set the size (in bits) of internal BLIS integers and" )
	print( "                 integer types used in native BLIS interfaces. The" )
	print( "                 default integer type size is architecture dependent." )
	print( "                 (Hint: You can always find this value printed at the" )
	print( "                 beginning of the testsuite output.)" )
	print( " " )
	print( "   -DBLAS_TYPE_SIZE=SIZE" )
	print( " " )
	print( "                 Set the size (in bits) of integer types in external" )
	print( "                 BLAS and CBLAS interfaces, if enabled. The default" )
	print( "                 integer type size used in BLAS/CBLAS is 32 bits." )
	print( " " )
	print( "   -DENABLE_BLAS=ON or -DENABLE_BLAS=OFF" )
	print( " " )
	print( "                 Disable (enabled by default) building the BLAS" )
	print( "                 compatibility layer." )
	print( " " )
	print( "   -DENABLE_CBLAS=ON or -DENABLE_CBLAS=OFF" )
	print( " " )
	print( "                 Enable (disabled by default) building the CBLAS" )
	print( "                 compatibility layer. This automatically enables the" )
	print( "                 BLAS compatibility layer as well." )
	print( " " )
	print( "   -DENABLE_MIXED_DT=ON or -DENABLE_MIXED_DT=OFF" )
	print( " " )
	print( "                 Disable (enabled by default) support for mixing the" )
	print( "                 storage domain and/or storage precision of matrix" )
	print( "                 operands for the gemm operation, as well as support" )
	print( "                 for computing in a precision different from one or" )
	print( "                 both of matrices A and B." )
	print( " " )
	print( "   -DENABLE_MIXED_DT_EXTRA_MEM=ON or -DENABLE_MIXED_DT_EXTRA_MEM=OFF" )
	print( " " )
	print( "                 Disable (enabled by default) support for additional" )
	print( "                 mixed datatype optimizations that require temporarily" )
	print( "                 allocating extra memory--specifically, a single m x n" )
	print( "                 matrix (per application thread) whose storage datatype" )
	print( "                 is equal to the computation datatype. This option may" )
	print( "                 only be enabled when mixed domain/precision support is" )
	print( "                 enabled." )
	print( " " )
	print( "   -DENABLE_SUP_HANDLING=ON or -DENABLE_SUP_HANDLING=OFF" )
	print( " " )
	print( "                 Disable (enabled by default) handling of small/skinny" )
	print( "                 matrix problems via separate code branches. When disabled," )
	print( "                 these small/skinny level-3 operations will be performed by" )
	print( "                 the conventional implementation, which is optimized for" )
	print( "                 medium and large problems. Note that what qualifies as" )
	print( "                 \"small\" depends on thresholds that may vary by sub-" )
	print( "                 configuration." )
	print( " " )
	print( "   -DENABLE_ADDON=\"NAME1[;NAME2;...]\" (Linux only)")
	print( " " )
	print( "                 Enable the code provided by an addon. An addon consists" )
	print( "                 of a separate directory of code that provides additional" )
	print( "                 APIs, implementations, and/or operations that would" )
	print( "                 otherwise not be present within a build of BLIS." )
	print( "                 To enable a single addon named NAME1, set -DENABLE_ADDON=NAME1." )
	print( "                 To enable multiple addons, a ';'-separated list enclosed in \"\"")
	print( "                 needs to be provided. For example, -DENABLE_ADDON=\"NAME1;NAME2\".")
	print("                  By default, no addons are enabled.")
	print( " " )
	# Sandbox functionality is currently disabled in CMake.
	#print( "   -DENABLE_SANDBOX=NAME" )
	#print( " " )
	#print( "                 Enable a separate sandbox implementation of gemm. This" )
	#print( "                 option disables BLIS's conventional gemm implementation" )
	#print( "                 (which shares common infrastructure with other level-3" )
	#print( "                 operations) and instead compiles and uses the code in" )
	#print( "                 the NAME directory, which is expected to be a sub-" )
	#print( "                 directory of 'sandbox'. By default, no sandboxes are" )
	#print( "                 enabled." )
	#print( " " )
	print( "   -DENABLE_MEMKIND=ON or -DENABLE_MEMKIND=OFF" )
	print( " " )
	print( "                 Forcibly enable or disable the use of libmemkind's" )
	print( "                 hbw_malloc() and hbw_free() as substitutes for malloc()" )
	print( "                 and free(), respectively, when allocating memory for" )
	print( "                 BLIS's memory pools, which are used to manage buffers" )
	print( "                 into which matrices are packed. The default behavior" )
	print( "                 for this option is environment-dependent; if configure" )
	print( "                 detects the presence of libmemkind, libmemkind is used" )
	print( "                 by default, and otherwise it is not used by default." )
	print( " " )
	print( "   -DTHREAD_PART_JRIR=METHOD" )
	print( " " )
	print( "                 Request a method of assigning micropanels to threads in" )
	print( "                 the JR and IR loops. Valid values for METHOD are 'slab'" )
	print( "                 and 'rr'. Using 'slab' assigns (as much as possible)" )
	print( "                 contiguous regions of micropanels to each thread while" )
	print( "                 using 'rr' assigns micropanels to threads in a round-" )
	print( "                 robin fashion. The chosen method also applies during" )
	print( "                 the packing of A and B. The default method is 'slab'." )
	print( "                 NOTE: Specifying this option constitutes a request," )
	print( "                 which may be ignored in select situations if the" )
	print( "                 implementation has a good reason to do so." )
	print( " " )
	print( "   -DENABLE_TRSM_PREINVERSION=ON or -DENABLE_TRSM_PREINVERSION=OFF" )
	print( " " )
	print( "                 Disable (enabled by default) pre-inversion of triangular" )
	print( "                 matrix diagonals when performing trsm. When pre-inversion" )
	print( "                 is enabled, diagonal elements are inverted outside of the" )
	print( "                 microkernel (e.g. during packing) so that the microkernel" )
	print( "                 can use multiply instructions. When disabled, division" )
	print( "                 instructions are used within the microkernel. Executing" )
	print( "                 these division instructions within the microkernel will" )
	print( "                 incur a performance penalty, but numerical robustness will" )
	print( "                 improve for certain cases involving denormal numbers that" )
	print( "                 would otherwise result in overflow in the pre-inverted" )
	print( "                 values." )
	print( " " )
	print( "   -DFORCE_VERSION_STRING=STRING" )
	print( " " )
	print( "                 Force configure to use an arbitrary version string" )
	print( "                 STRING. This option may be useful when repackaging" )
	print( "                 custom versions of BLIS by outside organizations." )
	print( " " )
	print( "   -DCOMPLEX_RETURN=gnu or -DCOMPLEX_RETURN=intel or -DCOMPLEX_RETURN=default" )
	print( " " )
	print( "                 Specify the way in which complex numbers are returned" )
	print( "                 from Fortran functions, either \"gnu\" (return in" )
	print( "                 registers) or \"intel\" (return via hidden argument)." )
	print( "                 By default COMPLEX_RETURNis set to 'default' and we" )
	print( "                 attempt to determine the return type from the compiler." )
	print( "                 Otherwise, the default is \"gnu\"." )
	print( " " )
	print( "   -DENABLE_AOCL_DYNAMIC=ON or -DENABLE_AOCL_DYNAMIC=OFF" )
	print( " " )
	print( "                 Disable (Enabled by default) dynamic selection of number of" )
	print( "                 threads used to solve the given problem." )
	print( "                 Range of optimum number of threads will be [1, num_threads]," )
	print( "                 where \"num_threads\" is number of threads set by the application." )
	print( "                 Num_threads is derived from either environment variable" )
	print( "                 OMP_NUM_THREADS or BLIS_NUM_THREADS' or bli_set_num_threads() API." )
	print( " " )
	print( "   -DDISABLE_BLIS_ARCH_TYPE=ON or -DDISABLE_BLIS_ARCH_TYPE=OFF" )
	print( " " )
	print( "                 Disable support for AOCL_ENABLE_INSTRUCTIONS, BLIS_ARCH_TYPE and" )
	print( "                 BLIS_MODEL_TYPE environment variables, which allows user to select" )
	print( "                 architecture specific code path and optimizations at runtime." )
	print( "                 If disabled, in builds with multiple code paths, BLIS" )
	print( "                 will still select path and optimizations automatically." )
	print( "                 Default: Enabled in builds with multiple code paths, else disabled." )
	print( " " )
	print( "   -DRENAME_BLIS_ARCH_TYPE=STRING" )
	print( " " )
	print( "                 Change environment variable used to select architecture specific" )
	print( "                 code path from BLIS_ARCH_TYPE to STRING" )
	print( " " )
	print( "   -DRENAME_BLIS_MODEL_TYPE=STRING" )
	print( " " )
	print( "                 Change environment variable used to select architecture model specific" )
	print( "                 optimizations from BLIS_MODEL_TYPE to STRING" )
	print( " " )
	print( "   -DENABLE_NO_UNDERSCORE_API=OFF" )
	print( " " )
	print( "                 Export APIs without underscore" )
	print( " " )
	print( "   -DENABLE_UPPERCASE_API=OFF" )
	print( " " )
	print( "                 Export APIs with uppercase" )
	print( " " )
	print( "   -DENABLE_COVERAGE=ON or -DENABLE_COVERAGE=OFF" )
	print( " " )
	print( "                 Enable (disabled by default) generation of code coverage" )
	print( "                 report in html format. Code coverage support is provided" )
	print( "                 only on LINUX with GCC compiler." )
	print( " " )
	print( "   -DENABLE_ASAN=ON or -DENABLE_ASAN=OFF" )
	print( " " )
	print( "                 Enable (disabled by default) Address Sanitizer to find " )
	print( "                 memory access error. Address Sanitizer support is provided" )
	print( "                 only on LINUX with Clang compiler" )
	print( " " )
	print( " Additional CMake Variables:" )
	print( " " )
	print( "   CMAKE_C_COMPILER            Specifies the C compiler to use." )
	print( "   CMAKE_CXX_COMPILER          Specifies the C++ compiler to use (sandbox only)." )
	print( "   CMAKE_Fortran_COMPILER      Specifies the Fortran compiler to use (only to determine --complex-return)." )
	print( "   COMPILE_OPTIONS             Specifies additional compiler flags to use." )
	print( "   COMPILE_DEFINITIONS         Specifies additional preprocessor definitions to use." )
	print( "   LINK_OPTIONS                Specifies additional linker flags to use." )
	print( " " )
	print( "   Note that not all compilers are compatible with a given" )
	print( "   configuration." )

	# Return from main().
	return 0


if __name__ == "__main__":
	main()
