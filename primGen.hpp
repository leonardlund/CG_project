void Assignment07::createRedLineMesh(std::vector<VertexGenerated>& vDef, std::vector<uint32_t>& vIdx) {
	vDef.push_back({ {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}}); //0
	vDef.push_back({ {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}}); //1
	vDef.push_back({ {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}}); //2
	vDef.push_back({ {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}}); //3

	// front
	vIdx.push_back(0); vIdx.push_back(1); vIdx.push_back(2);
	vIdx.push_back(1); vIdx.push_back(2); vIdx.push_back(3);
}
