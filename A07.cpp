// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"



// The uniform buffer object used in this example
struct UniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightPosition;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
	alignas(16) glm::vec3 lightDirDoll;
	alignas(16) glm::vec4 lightColorDoll;
	alignas(16) glm::vec3 eyePosDoll;
};

struct VertexGenerated {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec3 color;
};

struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

class Assignment07;
void GameLogic(Assignment07 *A, float Ar, glm::mat4 &ViewPrj, glm::mat4 &World, glm::vec3 &ViewPosition, float &dollAngle);

// MAIN ! 
class Assignment07 : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSLMesh, DSLGenerated;

	// Vertex formats
	VertexDescriptor VMesh, VGenerated;

	// Pipelines [Shader couples]
	Pipeline PMesh, PGenerated;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model<VertexMesh> M1, M2, MG;
	Model<VertexGenerated> ModelRedLine;
	Texture T1, T2, TG[4], TRedLine;
	DescriptorSet DS1, DS2, DSG[4], DSRedLine;
	
	// Other application parameters
	float Ar;
	glm::mat4 GWM[4];

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Assignment 07";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.0f, 0.6f, 0.8f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 13;
		texturesInPool = 7;
		setsInPool = 7;
		
		Ar = 4.0f / 3.0f;
	}
	
	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		std::cout << "Window resized to: " << w << " x " << h << "\n";
		Ar = (float)w / (float)h;
	}
	
	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLMesh.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the type of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
					{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
				  });

		DSLGenerated.init(this, {
			// this array contains the binding:
			// first  element : the binding number
			// second element : the type of element (buffer or texture)
			// third  element : the pipeline stage where it will be used
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});

		VMesh.init(this, {
			// this array contains the bindings
			// first  element : the binding number
			// second element : the stride of this binging
			// third  element : whether this parameter change per vertex or per instance
			//                  using the corresponding Vulkan constant
			{0, sizeof(VertexMesh), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				// this array contains the location
				// first  element : the binding number
				// second element : the location number
				// third  element : the offset of this element in the memory record
				// fourth element : the data type of the element
				//                  using the corresponding Vulkan constant
				// fifth  elmenet : the size in byte of the element
				// sixth  element : a constant defining the element usage
				//                   POSITION - a vec3 with the position
				//                   NORMAL   - a vec3 with the normal vector
				//                   UV       - a vec2 with a UV coordinate
				//                   COLOR    - a vec4 with a RGBA color
				//                   TANGENT  - a vec4 with the tangent vector
				//                   OTHER    - anything else
				//
				// ***************** DOUBLE CHECK ********************
				//    That the Vertex data structure you use in the "offsetoff" and
				//	in the "sizeof" in the previous array, refers to the correct one,
				//	if you have more than one vertex format!
				// ***************************************************
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMesh, UV),
					   sizeof(glm::vec2), UV}
			});

		VGenerated.init(this, {
			{0, sizeof(VertexGenerated), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexGenerated, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexGenerated, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexGenerated, color),
					   sizeof(glm::vec3), COLOR}
			});


		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		PMesh.init(this, &VMesh, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", {&DSLMesh});
		PMesh.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
 								    VK_CULL_MODE_NONE, false);
		PGenerated.init(this, &VGenerated, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", { &DSLGenerated });
		// PGenerated.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
		//	VK_CULL_MODE_NONE, false);


		// Models, textures and Descriptors (values assigned to the uniforms)
		M1.init(this, &VMesh, "models/Character.obj", OBJ);
		M2.init(this, &VMesh, "models/doll.obj", OBJ);
		MG.init(this, &VMesh, "models/floor.obj", OBJ);
		createRedLineMesh(ModelRedLine.vertices, ModelRedLine.indices);
		ModelRedLine.initMesh(this, &VGenerated);
		
		T1.init(this, "textures/Colors2.png");
		T2.init(this, "textures/Material.001_baseColor.png");
		TG[0].init(this, "textures/None_baseColor.jpeg");
		TG[1].init(this, "textures/None_baseColor.jpeg");
		TG[2].init(this, "textures/None_baseColor.jpeg");
		TG[3].init(this, "textures/None_baseColor.jpeg");
		

		GWM[0] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(-1,0,-1));	
		GWM[1] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(0,0,-1));	
		GWM[2] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(-1,0,0));	
		GWM[3] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(0,0,0));	
	}
	
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		PMesh.create();
		PGenerated.create();


		DS1.init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T1},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
				});
		DS2.init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T2},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		for(int i = 0; i < 4; i++) {
			DSG[i].init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &TG[i]},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
				});
		}
		DSRedLine.init(this, &DSLGenerated, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
	}

	// Here you destroy your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsCleanup() {
		PMesh.cleanup();
		PGenerated.cleanup();

		
		DS1.cleanup();
		DS2.cleanup();
		DSG[0].cleanup();
		DSG[1].cleanup();
		DSG[2].cleanup();
		DSG[3].cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// You also have to destroy the pipelines
	void localCleanup() {
		T1.cleanup();
		T2.cleanup();
		M1.cleanup();
		M2.cleanup();
		ModelRedLine.cleanup();
		TG[0].cleanup();
		TG[1].cleanup();
		TG[2].cleanup();
		TG[3].cleanup();
		MG.cleanup();

		DSLMesh.cleanup();
		DSLGenerated.cleanup();
		
		PMesh.destroy();		
		PGenerated.destroy();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

		PMesh.bind(commandBuffer);
		PGenerated.bind(commandBuffer);

		M1.bind(commandBuffer);
		
		DS1.bind(commandBuffer, PMesh, 0, currentImage);
		
					
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);
		
		/* Leo addition */
		M2.bind(commandBuffer);
		DS2.bind(commandBuffer, PMesh, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(M2.indices.size()), 1, 0, 0, 0);
		/* End Leo addition*/

		MG.bind(commandBuffer);
		for(int i = 0; i < 4; i++) {
			DSG[i].bind(commandBuffer, PMesh, 0, currentImage);
						
			vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(MG.indices.size()), 1, 0, 0, 0);
		}
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		glm::mat4 ViewPrj;
		glm::mat4 WM;
		glm::vec3 ViewPosition;
		float dollAngle;
		
		GameLogic(this, Ar, ViewPrj, WM, ViewPosition, dollAngle);
		
		glm::quat dollRotationQuaternion = glm::quat(glm::vec3(0, dollAngle, 0));

		UniformBufferObject ubo{};								
		// Here is where you actually update your uniforms

		// updates global uniforms
		GlobalUniformBufferObject gubo{};
		//gubo.lightPosition= glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
		gubo.lightPosition = glm::vec3(100.0f, 100.0f, 100.0f);
		gubo.lightColor = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);
		gubo.eyePos = ViewPosition;
		/* Leo addition */
		gubo.lightDirDoll = glm::vec3(sin(dollAngle), -sin(glm::radians(-30.0f)), cos(dollAngle));
		gubo.lightColorDoll = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		gubo.eyePosDoll = glm::vec3(0, 1, 0);
		/* End Leo addition */

		// CHARACTER UBO
		ubo.mMat = WM * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0,1,0));
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		DS1.map(currentImage, &ubo, sizeof(ubo), 0);
		DS1.map(currentImage, &gubo, sizeof(gubo), 2);

		// DOLL UBO
		ubo.mMat = glm::translate(glm::scale(glm::mat4(1), glm::vec3(2)), glm::vec3(0, 0, 0)) * glm::mat4(dollRotationQuaternion);
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		DS2.map(currentImage, &ubo, sizeof(ubo), 0);
		DS2.map(currentImage, &gubo, sizeof(gubo), 2);

		for(int i = 0; i < 4; i++) {
			ubo.mMat = GWM[i];
			ubo.mvpMat = ViewPrj * ubo.mMat;
			ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
			DSG[i].map(currentImage, &ubo, sizeof(ubo), 0);
			DSG[i].map(currentImage, &gubo, sizeof(gubo), 2);
		}
	}
	void createRedLineMesh(std::vector<VertexGenerated> &vDef, std::vector<uint32_t> &vIdx);

};
#include "primGen.hpp"
#include "Logic.hpp"

// This is the main: probably you do not need to touch this!
int main() {
    Assignment07 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}