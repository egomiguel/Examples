#include "PrivateData.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

static void DeallocateBuffer(void* data, size_t) {
    std::free(data);
}

static TF_Buffer* ReadBufferFromFile(const char* file) {
    std::ifstream f(file, std::ios::binary);
    //SCOPE_EXIT{ f.close(); };
    if (f.fail() || !f.is_open()) {
        return nullptr;
    }

    if (f.seekg(0, std::ios::end).fail()) {
        return nullptr;
    }
    auto fsize = f.tellg();
    if (f.seekg(0, std::ios::beg).fail()) {
        return nullptr;
    }

    if (fsize <= 0) {
        return nullptr;
    }

    auto data = static_cast<char*>(std::malloc(fsize));
    if (f.read(data, fsize).fail()) {
        return nullptr;
    }

    f.close();

    auto buf = TF_NewBuffer();
    buf->data = data;
    buf->length = fsize;
    buf->data_deallocator = DeallocateBuffer;

    return buf;
}

PrivateData::PrivateData()
{
    mGraph = nullptr;
    mSession = nullptr;
}

PrivateData::~PrivateData()
{
    TF_DeleteGraph(mGraph);
    DeleteSession(mSession);
}

bool PrivateData::init(const std::string& pModel)
{
    if (mGraph != nullptr)
    {
        TF_DeleteGraph(mGraph);
    }

    DeleteSession(mSession);

    TF_Status* status = TF_NewStatus();
    LoadGraph(pModel.c_str(), status);

    if (TF_GetCode(status) != TF_OK)
    {
        return false;
    }

    CreateSession(mGraph, status);

    if (TF_GetCode(status) != TF_OK)
    {
        return false;
    }

    return true;
}

TF_Tensor* PrivateData::CreateEmptyTensor(TF_DataType data_type, const std::int64_t* dims, std::size_t num_dims, std::size_t len)
{
    if (dims == nullptr) {
        return nullptr;
    }

    return TF_AllocateTensor(data_type, dims, static_cast<int>(num_dims), len);
}

TF_Tensor* PrivateData::CreateEmptyTensor(TF_DataType data_type, const std::vector<std::int64_t>& dims, std::size_t len)
{
    return CreateEmptyTensor(data_type, dims.data(), dims.size(), len);
}

TF_Tensor* PrivateData::CreateTensor(TF_DataType data_type, const std::int64_t* dims, std::size_t num_dims, const void* data, std::size_t len)
{
    auto tensor = CreateEmptyTensor(data_type, dims, num_dims, len);
    if (tensor == nullptr) {
        return nullptr;
    }

    auto tensor_data = TF_TensorData(tensor);
    if (tensor_data == nullptr) {
        if (tensor != nullptr)
        {
            TF_DeleteTensor(tensor);
        }
        return nullptr;
    }

    len = std::min(len, TF_TensorByteSize(tensor));
    if (data != nullptr && len != 0) {
        std::memcpy(tensor_data, data, len);
    }

    return tensor;
}

void PrivateData::LoadGraph(const char* graph_path, TF_Status* status)
{
    if (graph_path == nullptr) {
        if (mGraph != nullptr)
        {
            TF_DeleteGraph(mGraph);
        }
        return;
    }

    auto buffer = ReadBufferFromFile(graph_path);
    if (buffer == nullptr) {
        if (mGraph != nullptr)
        {
            TF_DeleteGraph(mGraph);
        }
        return;
    }

    if (status == nullptr) {
        status = TF_NewStatus();
    }

    mGraph = TF_NewGraph();
    auto opts = TF_NewImportGraphDefOptions();

    TF_GraphImportGraphDef(mGraph, buffer, opts, status);
    TF_DeleteImportGraphDefOptions(opts);
    TF_DeleteBuffer(buffer);

    if (TF_GetCode(status) != TF_OK) {
        if (mGraph != nullptr) {
            TF_DeleteGraph(mGraph);
        }
        return;
    }
}

void PrivateData::CreateSession(TF_Graph* graph, TF_Status* status)
{
    if (graph == nullptr) {
        DeleteSession(mSession);
        return;
    }

    if (status == nullptr) {
        status = TF_NewStatus();
    }


    TF_SessionOptions* options = TF_NewSessionOptions();

    mSession = TF_NewSession(graph, options, status);

    if (TF_GetCode(status) != TF_OK) {
        DeleteSession(mSession);
    }
}

TF_Code PrivateData::DeleteSession(TF_Session* session, TF_Status* status)
{
    if (session == nullptr) {
        return TF_INVALID_ARGUMENT;
    }

    if (status == nullptr) {
        status = TF_NewStatus();
    }

    TF_CloseSession(session, status);
    TF_DeleteSession(session, status);
    return TF_GetCode(status);
}

TF_Code PrivateData::RunSession(TF_Session* session,
    const TF_Output* inputs, TF_Tensor* const* input_tensors, std::size_t ninputs,
    const TF_Output* outputs, TF_Tensor** output_tensors, std::size_t noutputs,
    TF_Status* status)
{
    if (session == nullptr ||
        inputs == nullptr || input_tensors == nullptr ||
        outputs == nullptr || output_tensors == nullptr) {
        return TF_INVALID_ARGUMENT;
    }

    if (status == nullptr) {
        status = TF_NewStatus();
    }

    TF_SessionRun(session,
        nullptr, // Run options.
        inputs, input_tensors, static_cast<int>(ninputs), // Input tensors, input tensor values, number of inputs.
        outputs, output_tensors, static_cast<int>(noutputs), // Output tensors, output tensor values, number of outputs.
        nullptr, 0, // Target operations, number of targets.
        nullptr, // Run metadata.
        status // Output status.
    );

    return TF_GetCode(status);
}

TF_Code PrivateData::Execute(const std::vector<TF_Output>& inputs, const std::vector<TF_Tensor*>& input_tensors,
    const std::vector<TF_Output>& outputs, std::vector<TF_Tensor*>& output_tensors)
{
    return RunSession(mSession, inputs.data(), input_tensors.data(), input_tensors.size(),
        outputs.data(), output_tensors.data(), output_tensors.size());
}

TF_Tensor* PrivateData::CreateTensor(TF_DataType data_type, const std::vector<std::int64_t>& dims, const void* data, std::size_t dataLen)
{
    return CreateTensor(data_type, dims.data(), dims.size(), data, dataLen);
}


TF_Graph* PrivateData::getGraph() const
{
    return mGraph;
}

TF_Session* PrivateData::getSession() const
{
    return mSession;
}
