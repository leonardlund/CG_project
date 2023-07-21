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

struct OverlayUniformBlock {
	alignas(4) float visible;
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

struct VertexOverlay {
	glm::vec2 pos;
	glm::vec2 UV;
};

class Assignment07;
void GameLogic(Assignment07 *A, float Ar, glm::mat4 &ViewPrj, glm::mat4 &World, glm::vec3 &ViewPosition, float &dollAngle, int &gameState);

// MAIN ! 
class Assignment07 : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSLMesh, DSLOverlay;

	// Vertex formats
	VertexDescriptor VMesh, VOverlay;

	// Pipelines [Shader couples]
	Pipeline PMesh, POverlay;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model<VertexMesh> M1, M2, MG, MRedLine, MW;
	Model<VertexOverlay> MSplash;

	//Model<VertexGenerated> ModelRedLine;
	Texture T1, T2, TG[4], TRedLine, TSplash, TWinSplash, TLostSplash, TW[4];
	DescriptorSet DS1, DS2, DSG[4], DSRedLine, DSSplash, DSWinSplash, DSLostSplash, DSW[4];

	OverlayUniformBlock uboSplash;

	
	// Other application parameters
	float Ar;
	glm::mat4 GWM[4];
    glm::mat4 WWM[4];

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Assignment 07";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.0f, 0.6f, 0.8f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 28;
		texturesInPool = 23;
		setsInPool = 23;
		
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
		DSLOverlay.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
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
		VOverlay.init(this, {
				  {0, sizeof(VertexOverlay), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
			  {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, pos),
					 sizeof(glm::vec2), OTHER},
			  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, UV),
					 sizeof(glm::vec2), UV}
			});
		


		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		PMesh.init(this, &VMesh, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", {&DSLMesh});
		PMesh.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
 								    VK_CULL_MODE_NONE, false);
		POverlay.init(this, &VOverlay, "shaders/OverlayVert.spv", "shaders/OverlayFrag.spv", { &DSLOverlay });
		POverlay.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, false);

		// Models, textures and Descriptors (values assigned to the uniforms)
		M1.init(this, &VMesh, "models/Character.obj", OBJ);
		M2.init(this, &VMesh, "models/doll.obj", OBJ);
		MG.init(this, &VMesh, "models/floor.obj", OBJ);
        MRedLine.init(this, &VMesh, "models/floor.obj", OBJ);
		MW.init(this, &VMesh, "models/floor.obj", OBJ);
		MSplash.vertices = { {{-1.0f, -1.0f}, {0.0102f, 0.0f}}, {{-1.0f, 1.0f}, {0.0102f,0.85512f}},
						 {{ 1.0f,-1.0f}, {1.0f,0.0f}}, {{ 1.0f, 1.0f}, {1.0f,0.85512f}} };
		MSplash.indices = { 0, 1, 2,    1, 2, 3 };
		MSplash.initMesh(this, &VOverlay);


		T1.init(this, "textures/CharacterTexture.png");
		T2.init(this, "textures/DollTexture.png");
		TG[0].init(this, "textures/GroundTexture.jpeg");
		TG[1].init(this, "textures/GroundTexture.jpeg");
		TG[2].init(this, "textures/GroundTexture.jpeg");
		TG[3].init(this, "textures/GroundTexture.jpeg");
		TW[0].init(this, "textures/wall.jpg");
		TW[1].init(this, "textures/wall.jpg");
		TW[2].init(this, "textures/wall.jpg");
		TW[3].init(this, "textures/wall.jpg");
        TRedLine.init(this, "textures/RedColor.jpeg");
		TSplash.init(this, "textures/SplashScreen.png");
		TWinSplash.init(this, "textures/YouWon.png");
		TLostSplash.init(this, "textures/YouDied.png");

		

		// GROUND World matrix
		GWM[0] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(-1,0,-1));
		GWM[1] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(0,0,-1));	
		GWM[2] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(-1,0,0));	
		GWM[3] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(0,0,0));
        
		// WALL world matrix
        //right
        WWM[0] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(-80, 10, 100)),glm::vec3(-0.2,0,0.3)) * glm::inverse(glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1,0,0)));
        //left
        WWM[1] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(-80, 10, -100)),glm::vec3(-0.2,0,0.3)) * glm::inverse(glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1,0,0)));
        //front
        WWM[2] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(-50, 10, 150)),glm::vec3(-0.2,0,-0.5)) * glm::inverse(glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0,0,-1)));
        //back
        WWM[3] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(-50, 10, 150)),glm::vec3(1.2,0,-0.5)) * glm::inverse(glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0,0,-1)));

	}
	
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		PMesh.create();
		POverlay.create();


		// PLAYER
		DS1.init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T1},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });
        
		// DOLL
        DS2.init(this, &DSLMesh, {
            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
            {1, TEXTURE, 0, &T2},
            {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });
        
		// FINISH LINE
        DSRedLine.init(this, &DSLMesh, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &TRedLine},
                    {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
        });
        
		// GROUND
		for(int i = 0; i < 4; i++) {
			DSG[i].init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &TG[i]},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
				});
		}
		DSSplash.init(this, &DSLOverlay, {
					{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
					{1, TEXTURE, 0, &TSplash}
			});
		DSWinSplash.init(this, &DSLOverlay, {
					{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
					{1, TEXTURE, 0, &TWinSplash}
			});

		DSLostSplash.init(this, &DSLOverlay, {
					{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
					{1, TEXTURE, 0, &TLostSplash}
			});
	
        // WALLS
        for(int i = 0; i < 4; i++) {
            DSW[i].init(this, &DSLMesh, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &TW[i]},
                    {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
                });
        }
	}

	// Here you destroy your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsCleanup() {
		PMesh.cleanup();
		POverlay.cleanup();

		DS1.cleanup();
		DS2.cleanup();
		DSG[0].cleanup();
		DSG[1].cleanup();
		DSG[2].cleanup();
		DSG[3].cleanup();
        DSRedLine.cleanup();
		DSSplash.cleanup();
		DSWinSplash.cleanup();
		DSLostSplash.cleanup();

        DSW[0].cleanup();
        DSW[1].cleanup();
        DSW[2].cleanup();
        DSW[3].cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// You also have to destroy the pipelines
	void localCleanup() {
		T1.cleanup();
		T2.cleanup();
		TSplash.cleanup();
		TWinSplash.cleanup();
		TLostSplash.cleanup();
		M1.cleanup();
		M2.cleanup();
		TG[0].cleanup();
		TG[1].cleanup();
		TG[2].cleanup();
		TG[3].cleanup();
		MG.cleanup();
        TRedLine.cleanup();
        MRedLine.cleanup();
        TW[0].cleanup();
        TW[1].cleanup();
        TW[2].cleanup();
        TW[3].cleanup();
        MW.cleanup();
		MSplash.cleanup();

		DSLMesh.cleanup();
		
		PMesh.destroy();		
		POverlay.destroy();

	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        
        PMesh.bind(commandBuffer);

		// PLAYER
        M1.bind(commandBuffer);
        DS1.bind(commandBuffer, PMesh, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);
        
        // DOLL
        M2.bind(commandBuffer);
        DS2.bind(commandBuffer, PMesh, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M2.indices.size()), 1, 0, 0, 0);
        
		// FINISH LINE
        MRedLine.bind(commandBuffer);
        DSRedLine.bind(commandBuffer, PMesh, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MRedLine.indices.size()), 1, 0, 0, 0);

		MG.bind(commandBuffer);
		for(int i = 0; i < 4; i++) {
			DSG[i].bind(commandBuffer, PMesh, 0, currentImage);
						
			vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(MG.indices.size()), 1, 0, 0, 0);
		}
		MW.bind(commandBuffer);
		for (int i = 0; i < 4; i++) {
			DSW[i].bind(commandBuffer, PMesh, 0, currentImage);

			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MW.indices.size()), 1, 0, 0, 0);
		}

		POverlay.bind(commandBuffer);
		MSplash.bind(commandBuffer);
		DSSplash.bind(commandBuffer, POverlay, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MSplash.indices.size()), 1, 0, 0, 0);

		DSWinSplash.bind(commandBuffer, POverlay, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MSplash.indices.size()), 1, 0, 0, 0);
		DSLostSplash.bind(commandBuffer, POverlay, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MSplash.indices.size()), 1, 0, 0, 0);
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		glm::mat4 ViewPrj;
		glm::mat4 WM;
		glm::vec3 ViewPosition;
		float dollAngle;
		static int gameState = 0;

		GameLogic(this, Ar, ViewPrj, WM, ViewPosition, dollAngle, gameState);

		glm::quat dollRotationQuaternion = glm::quat(glm::vec3(0, -dollAngle + glm::radians(90.0f), 0));

		UniformBufferObject ubo{};
		// Here is where you actually update your uniforms

		// updates global uniforms
		GlobalUniformBufferObject gubo{};
		gubo.lightPosition = glm::vec3(100.0f, 100.0f, 100.0f);
		gubo.lightColor = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);
		gubo.eyePos = ViewPosition;
		/* Leo addition */
		gubo.lightDirDoll = glm::vec3(cos(dollAngle), sin(glm::radians(-15.0f)), sin(dollAngle));
		gubo.lightColorDoll = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		gubo.eyePosDoll = glm::vec3(0, 0.2, 0);
		/* End Leo addition */

		// CHARACTER UBO
		ubo.mMat = WM * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		DS1.map(currentImage, &ubo, sizeof(ubo), 0);
		DS1.map(currentImage, &gubo, sizeof(gubo), 2);

		// DOLL UBO
		ubo.mMat = glm::translate(glm::scale(glm::mat4(1), glm::vec3(1)), glm::vec3(0, 0, 0)) * glm::mat4(dollRotationQuaternion);
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		DS2.map(currentImage, &ubo, sizeof(ubo), 0);
		DS2.map(currentImage, &gubo, sizeof(gubo), 2);

		// REDLINE UBO
		ubo.mMat = glm::translate(glm::scale(glm::mat4(1), glm::vec3(3, 1, 100)), glm::vec3(-1, 0.01, -0.3));
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		DSRedLine.map(currentImage, &ubo, sizeof(ubo), 0);
		DSRedLine.map(currentImage, &gubo, sizeof(gubo), 2);

        // GROUND UBO
		for (int i = 0; i < 4; i++) {
			ubo.mMat = GWM[i];
			ubo.mvpMat = ViewPrj * ubo.mMat;
			ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
			DSG[i].map(currentImage, &ubo, sizeof(ubo), 0);
			DSG[i].map(currentImage, &gubo, sizeof(gubo), 2);
		}
		// WALL UBO
		for (int i = 0; i < 4; i++) {
			ubo.mMat = WWM[i];
			ubo.mvpMat = ViewPrj * ubo.mMat;
			ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
			DSW[i].map(currentImage, &ubo, sizeof(ubo), 0);
			DSW[i].map(currentImage, &gubo, sizeof(gubo), 2);
		}
        
		uboSplash.visible = (gameState == 0) ? 1.0f : 0.0f;
		DSSplash.map(currentImage, &uboSplash, sizeof(uboSplash), 0);

		uboSplash.visible = (gameState == 2) ? 1.0f : 0.0f;
		DSLostSplash.map(currentImage, &uboSplash, sizeof(uboSplash), 0);

		uboSplash.visible = (gameState == 3) ? 1.0f : 0.0f;
		DSWinSplash.map(currentImage, &uboSplash, sizeof(uboSplash), 0);

	}
};

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
