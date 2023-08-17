#ifndef PRIVATE_DATA_H
#define PRIVATE_DATA_H

#include "tensorflow/c/c_api.h"
#include <vector>

class PrivateData
{
private:
    TF_Tensor* CreateEmptyTensor(TF_DataType data_type, const std::int64_t* dims, std::size_t num_dims, std::size_t len = 0);
    TF_Tensor* CreateEmptyTensor(TF_DataType data_type, const std::vector<std::int64_t>& dims, std::size_t len = 0);
    TF_Tensor* CreateTensor(TF_DataType data_type, const std::int64_t* dims, std::size_t num_dims, const void* data, std::size_t len);
    TF_Code DeleteSession(TF_Session* session, TF_Status* status = nullptr);
    TF_Code RunSession(TF_Session* session,
        const TF_Output* inputs, TF_Tensor* const* input_tensors, std::size_t ninputs,
        const TF_Output* outputs, TF_Tensor** output_tensors, std::size_t noutputs,
        TF_Status* status = nullptr);

    void LoadGraph(const char* graph_path, TF_Status* status = nullptr);

    void CreateSession(TF_Graph* graph, TF_Status* status = nullptr);

    TF_Graph* mGraph;
    TF_Session* mSession;
public:
    PrivateData();
    ~PrivateData();

    bool init(const std::string& pModel);

    TF_Graph* getGraph() const;

    TF_Session* getSession() const;

    TF_Tensor* CreateTensor(TF_DataType data_type, const std::vector<std::int64_t>& dims, const void* data, std::size_t dataLen);
    
    TF_Code Execute(const std::vector<TF_Output>& inputs, const std::vector<TF_Tensor*>& input_tensors,
                       const std::vector<TF_Output>& outputs, std::vector<TF_Tensor*>& output_tensors);

    template <typename T>
    std::vector<T> GetTensorData(const TF_Tensor* tensor)
    {
        if (tensor == nullptr) {
            return {};
        }
        auto data = static_cast<T*>(TF_TensorData(tensor));
        auto size = TF_TensorByteSize(tensor) / TF_DataTypeSize(TF_TensorType(tensor));
        if (data == nullptr || size <= 0) {
            return {};
        }

        return { data, data + size };
    }
};

#endif