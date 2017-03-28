#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef _UTILS_H_
#define _UTILS_H_

#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

using namespace std;

void print_help() {
	std::cerr << "Application usage:" << std::endl;

	std::cerr << "  -p : select platform " << std::endl;
	std::cerr << "  -d : select device" << std::endl;
	std::cerr << "  -l : list all platforms and devices" << std::endl;
	std::cerr << "  -h : print this message" << std::endl;
}

void PrintPreferredWorkGroupSize(cl::Context& context, const cl::Kernel& k, unsigned int data_size, unsigned int local_size) {
	cl::Device device = context.getInfo<CL_CONTEXT_DEVICES>()[0];
	size_t preferred = k.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);
	size_t max_work_group_size = k.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device);

	std::cout << "Preferred Kernel Options: " << "\n\t";
	std::cout << "Preferred Work Group Size: " << preferred << "\n\t";
	std::cout << "Max Work Group Size: " << max_work_group_size << "\n\t";
	std::cout << "Global Size: " << data_size << "\n\t";
	std::cout << "Local Size: " << local_size << '\n' << std::endl;
}

unsigned int GetPreferredWorkGroupSize(cl::Context& context, const cl::Kernel& k) {
	cl::Device device = context.getInfo<CL_CONTEXT_DEVICES>()[0];
	return k.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);
}

void PrintBuildErrors(cl::Context& context, cl::Program& program){
	std::cerr << "Build Status: "
			  << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(context.getInfo<CL_CONTEXT_DEVICES>()[0])
			  << '\n';
	std::cerr << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(
			context.getInfo<CL_CONTEXT_DEVICES>()[0]) << '\n';
	std::cerr << "Build Log:\t "
			  << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(context.getInfo<CL_CONTEXT_DEVICES>()[0])
			  << std::endl;
}


enum ProfilingResolution {
	PROF_NS = 1,
	PROF_US = 1000,
	PROF_MS = 1000000,
	PROF_S = 1000000000
};

string GetFullProfilingInfo(const cl::Event& evnt, ProfilingResolution resolution) {
	stringstream sstream;

	sstream << "Queued: " << (evnt.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>() - evnt.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>()) / resolution;
	sstream << "\n\tSubmitted: " << (evnt.getProfilingInfo<CL_PROFILING_COMMAND_START>() - evnt.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>()) / resolution;
	sstream << "\n\tExecuted: " << (evnt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evnt.getProfilingInfo<CL_PROFILING_COMMAND_START>()) / resolution;
	sstream << "\n\tTotal: " << (evnt.getProfilingInfo<CL_PROFILING_COMMAND_END>() - evnt.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>()) / resolution;

	switch (resolution) {
		case PROF_NS: sstream << "ns"; break;
		case PROF_US: sstream << "us"; break;
		case PROF_MS: sstream << "ms"; break;
		case PROF_S: sstream << "s"; break;
		default: break;
	}

	return sstream.str();
}

void ProfilingInfo(cl::Event& prof_event) {
	std::cout << "Execution time: "
			  << prof_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
				 prof_event.getProfilingInfo<CL_PROFILING_COMMAND_START>() << "ns" << "\n\t"
			  << GetFullProfilingInfo(prof_event, ProfilingResolution::PROF_US) << '\n' << std::endl;
};

template <typename T>
ostream& operator<< (ostream& out, const vector<T>& v) {
	if (!v.empty()) {
		out << '[';
		copy(v.begin(), v.end(), ostream_iterator<T>(out, ", "));
		out << "\b\b]";
	}
	return out;
}

string GetPlatformName(int platform_id) {
	vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	return platforms[platform_id].getInfo<CL_PLATFORM_NAME>();
}

string GetDeviceName(int platform_id, int device_id) {
	vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	vector<cl::Device> devices;
	platforms[platform_id].getDevices((cl_device_type)CL_DEVICE_TYPE_ALL, &devices);
	return devices[device_id].getInfo<CL_DEVICE_NAME>();
}

const char *getErrorString(cl_int error) {
	switch (error) {
		// run-time and JIT compiler errors
	case 0: return "CL_SUCCESS";
	case -1: return "CL_DEVICE_NOT_FOUND";
	case -2: return "CL_DEVICE_NOT_AVAILABLE";
	case -3: return "CL_COMPILER_NOT_AVAILABLE";
	case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case -5: return "CL_OUT_OF_RESOURCES";
	case -6: return "CL_OUT_OF_HOST_MEMORY";
	case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case -8: return "CL_MEM_COPY_OVERLAP";
	case -9: return "CL_IMAGE_FORMAT_MISMATCH";
	case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case -11: return "CL_BUILD_PROGRAM_FAILURE";
	case -12: return "CL_MAP_FAILURE";
	case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case -15: return "CL_COMPILE_PROGRAM_FAILURE";
	case -16: return "CL_LINKER_NOT_AVAILABLE";
	case -17: return "CL_LINK_PROGRAM_FAILURE";
	case -18: return "CL_DEVICE_PARTITION_FAILED";
	case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		// compile-time errors
	case -30: return "CL_INVALID_VALUE";
	case -31: return "CL_INVALID_DEVICE_TYPE";
	case -32: return "CL_INVALID_PLATFORM";
	case -33: return "CL_INVALID_DEVICE";
	case -34: return "CL_INVALID_CONTEXT";
	case -35: return "CL_INVALID_QUEUE_PROPERTIES";
	case -36: return "CL_INVALID_COMMAND_QUEUE";
	case -37: return "CL_INVALID_HOST_PTR";
	case -38: return "CL_INVALID_MEM_OBJECT";
	case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case -40: return "CL_INVALID_IMAGE_SIZE";
	case -41: return "CL_INVALID_SAMPLER";
	case -42: return "CL_INVALID_BINARY";
	case -43: return "CL_INVALID_BUILD_OPTIONS";
	case -44: return "CL_INVALID_PROGRAM";
	case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
	case -46: return "CL_INVALID_KERNEL_NAME";
	case -47: return "CL_INVALID_KERNEL_DEFINITION";
	case -48: return "CL_INVALID_KERNEL";
	case -49: return "CL_INVALID_ARG_INDEX";
	case -50: return "CL_INVALID_ARG_VALUE";
	case -51: return "CL_INVALID_ARG_SIZE";
	case -52: return "CL_INVALID_KERNEL_ARGS";
	case -53: return "CL_INVALID_WORK_DIMENSION";
	case -54: return "CL_INVALID_WORK_GROUP_SIZE";
	case -55: return "CL_INVALID_WORK_ITEM_SIZE";
	case -56: return "CL_INVALID_GLOBAL_OFFSET";
	case -57: return "CL_INVALID_EVENT_WAIT_LIST";
	case -58: return "CL_INVALID_EVENT";
	case -59: return "CL_INVALID_OPERATION";
	case -60: return "CL_INVALID_GL_OBJECT";
	case -61: return "CL_INVALID_BUFFER_SIZE";
	case -62: return "CL_INVALID_MIP_LEVEL";
	case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
	case -64: return "CL_INVALID_PROPERTY";
	case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
	case -66: return "CL_INVALID_COMPILER_OPTIONS";
	case -67: return "CL_INVALID_LINKER_OPTIONS";
	case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

		// extension errors
	case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
	case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
	case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
	case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	default: return "Unknown OpenCL error";
	}
}

void CheckError(cl_int error) {
	if (error != CL_SUCCESS) {
		cerr << "OpenCL call failed with error " << getErrorString(error) << endl;
		exit(1);
	}
}

void AddSources(cl::Program::Sources& sources, const string& file_name) {
	//TODO: add file existence check
	ifstream file(file_name);
	string* source_code = new string(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
	sources.push_back(make_pair((*source_code).c_str(), source_code->length() + 1));
}

string ListPlatformsDevices() {

	stringstream sstream;
	vector<cl::Platform> platforms;

	cl::Platform::get(&platforms);

	sstream << "Found " << platforms.size() << " platform(s):" << endl;

	for (unsigned int i = 0; i < platforms.size(); i++)
	{
		sstream << "\nPlatform " << i << ", " << platforms[i].getInfo<CL_PLATFORM_NAME>() << ", version: " << platforms[i].getInfo<CL_PLATFORM_VERSION>();

		sstream << ", vendor: " << platforms[i].getInfo<CL_PLATFORM_VENDOR>() << endl;
		//		sstream << ", extensions: " << platforms[i].getInfo<CL_PLATFORM_EXTENSIONS>() << endl;

		vector<cl::Device> devices;

		platforms[i].getDevices((cl_device_type)CL_DEVICE_TYPE_ALL, &devices);

		sstream << "\n   Found " << devices.size() << " device(s):" << endl;

		for (unsigned int j = 0; j < devices.size(); j++)
		{
			sstream << "\n      Device " << j << ", " << devices[j].getInfo<CL_DEVICE_NAME>() << ", version: " << devices[j].getInfo<CL_DEVICE_VERSION>();

			sstream << ", vendor: " << devices[j].getInfo<CL_DEVICE_VENDOR>();
			cl_device_type device_type = devices[j].getInfo<CL_DEVICE_TYPE>();
			sstream << ", type: ";
			if (device_type & CL_DEVICE_TYPE_DEFAULT)
				sstream << "DEFAULT ";
			if (device_type & CL_DEVICE_TYPE_CPU)
				sstream << "CPU ";
			if (device_type & CL_DEVICE_TYPE_GPU)
				sstream << "GPU ";
			if (device_type & CL_DEVICE_TYPE_ACCELERATOR)
				sstream << "ACCELERATOR ";
			sstream << ", compute units: " << devices[j].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
			sstream << ", clock freq [MHz]: " << devices[j].getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
			sstream << ", max memory size [B]: " << devices[j].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
			sstream << ", max allocatable memory [B]: " << devices[j].getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();

			sstream << endl;
		}
	}
	sstream << "----------------------------------------------------------------" << endl;

	return sstream.str();
}

cl::Context GetContext(int platform_id, int device_id) {
	vector<cl::Platform> platforms;

	cl::Platform::get(&platforms);

	for (unsigned int i = 0; i < platforms.size(); i++)
	{
		vector<cl::Device> devices;
		platforms[i].getDevices((cl_device_type)CL_DEVICE_TYPE_ALL, &devices);

		for (unsigned int j = 0; j < devices.size(); j++)
		{
			if ((i == platform_id) && (j == device_id))
				return cl::Context({ devices[j] });
		}
	}

	return cl::Context();
}


#endif
#pragma clang diagnostic pop