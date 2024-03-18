#include "display.h"


namespace MiniRenderer {
	void Display::initialize_buffers() {
		mColorBuffer = new uint32_t[mWinWidth * mWinHeight];
		mColorBufferTexture = SDL_CreateTexture(
			mRenderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			mWinWidth,
			mWinHeight
		);
	}

	bool Display::initialize_window() {
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
			printf("Error: %s\n", SDL_GetError());
			return false;
		}

#ifdef SDL_HINT_IME_SHOW_UI
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

		SDL_DisplayMode display_mode;
		SDL_GetCurrentDisplayMode(0, &display_mode);
		mWinWidth = 1280;
		mWinHeight = 720;
		mWindow = SDL_CreateWindow(
			"MiniRenderer",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			mWinWidth,
			mWinHeight,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
		);

		if (mWindow == nullptr)
		{
			fprintf(stderr, "Error creating SDL window.\n");
			return false;
		}

		mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
		if (mRenderer == nullptr)
		{
			fprintf(stderr, "Error creating SDL renderer.\n");
			return false;
		}

		initialize_buffers();
		mSettings = std::make_unique<Settings>();
		mModel = std::make_unique<Model>("head.obj");

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		mIo = &ImGui::GetIO();
		mIo->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		ImGui::StyleColorsDark();

		// Setup SDLRenderer2 binding
		ImGui_ImplSDL2_InitForSDLRenderer(mWindow, mRenderer);
		ImGui_ImplSDLRenderer2_Init(mRenderer);

		mIsRunning = true;
		return true;
	}

	bool Display::is_running() {
		return mIsRunning;
	}

	void Display::process_input() {
		SDL_Event event;
		SDL_PollEvent(&event);
		switch (event.type) {
		case SDL_QUIT:
			mIsRunning = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				mIsRunning = false;
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				mWinWidth = event.window.data1;
				mWinHeight = event.window.data2;
				SDL_Log("Window %d resized to %dx%d", event.window.windowID, event.window.data1, event.window.data2);
				initialize_buffers();
			}
			else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				mIsRunning = false;
			}
			break;
		}
		ImGui_ImplSDL2_ProcessEvent(&event);


	}
	std::string filePathName;
	std::string filePath;

	glm::vec2 Display::project(glm::vec3 point)
	{
		float fov_factor = 1.0f / tan(mSettings->camera.fov / 2.0f);
		glm::vec2 projected_point = {
			(fov_factor * point.x) / point.z,
			(fov_factor * point.y) / point.z };
		return projected_point;
	}

	void Display::update() {
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();



		// TODO: Remove this line
		ImGui::ShowDemoWindow(nullptr);

		// Main settings window
		ImGui::Begin("Window settings");
		ImGui::Checkbox("Show renderer window", &mSettings->open_windows.show_renderer_window);
		ImGui::Checkbox("Show camera window", &mSettings->open_windows.show_camera_window);
		ImGui::End();

		// Rendering settings window
		if (mSettings && mSettings->open_windows.show_renderer_window) {
			ImGui::Begin("Rendering settings");
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / mIo->Framerate, mIo->Framerate);
			ImGui::Button("Render screenshot (TGA)");
			ImGui::SeparatorText("Model");
			/*
			ImGui::Checkbox("Show model", &mSettings->show_model);
			ImGui::Checkbox("Show wireframe", &mSettings->show_wireframe);
			ImGui::Checkbox("Show normals", &mSettings->show_normals);
			ImGui::Checkbox("Show bounding box", &mSettings->show_bounding_box);
			*/

			if (ImGui::Button("Open Model")) {
				IGFD::FileDialogConfig config;
				config.path = filePath;
				ImGuiFileDialog::Instance()->OpenDialog("ChooseObjFile", "Choose model (.obj)", ".obj", config);
			}
			// display
			if (ImGuiFileDialog::Instance()->Display("ChooseObjFile")) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
					filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
					mModel = std::make_unique<Model>(filePathName);
				}
				ImGuiFileDialog::Instance()->Close();
			}
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), filePathName.c_str());
			ImGui::SameLine();
			ImGui::Text("loaded");
			ImGui::Text("%d vertices, %d faces", mModel->nVerts(), mModel->nFaces());
			// ImGui::Text(mModel->name.c_str());
			/*
			ImGui::SeparatorText("Lighting");
						ImGui::Checkbox("Enable lighting", &mSettings->enable_lighting);
			ImGui::Checkbox("Enable textures", &mSettings->enable_textures);
						ImGui::SliderFloat3("Light position", &mSettings->light.position[0], -100.0f, 100.0f);
			ImGui::SliderFloat3("Light color", &mSettings->light.color[0], 0.0f, 1.0f);
						ImGui::SliderFloat("Ambient strength", &mSettings->light.ambient_strength, 0.0f, 1.0f);
			ImGui::SliderFloat("Diffuse strength", &mSettings->light.diffuse_strength, 0.0f, 1.0f);
						ImGui::SliderFloat("Specular strength", &mSettings->light.specular_strength, 0.0f, 1.0f);
			ImGui::SliderFloat("Specular shininess", &mSettings->light.specular_shininess, 0.0f, 100.0f);
						ImGui::SeparatorText("Shading");
			ImGui::RadioButton("Flat", reinterpret_cast<int*>(&mSettings->shading), Shading::Flat);
						ImGui::SameLine();
			ImGui::RadioButton("Gouraud", reinterpret_cast<int*>(&mSettings->shading), Shading::Gouraud);
						ImGui::SameLine();
			ImGui::RadioButton("Phong", reinterpret_cast<int*>(&mSettings->shading), Shading::Phong);
						ImGui::SeparatorText("Clipping");
			ImGui::Checkbox("Enable clipping", &mSettings->enable_clipping);
						ImGui::SliderFloat("Near plane", &mSettings->near_plane, 0.0f, 100.0f);
			ImGui::SliderFloat("Far plane", &mSettings->far_plane, 0.0f, 100.0f);
						ImGui::SeparatorText("Culling");
			ImGui::Checkbox("Enable backface culling", &mSettings->enable_backface_culling);
						ImGui::SeparatorText("Rasterization");
			ImGui::RadioButton("Point", reinterpret_cast<int*>(&mSettings->rasterization), Rasterization::Point);
						ImGui::SameLine();
			ImGui::RadioButton("Wireframe", reinterpret_cast<int*>(&mSettings->rasterization), Rasterization::Wireframe);
						ImGui::SameLine();
			ImGui::RadioButton("Solid", reinterpret_cast<int*>(&mSettings->rasterization), Rasterization::Solid);
			*/


			ImGui::SeparatorText("Line algorithm");
			ImGui::RadioButton("DDA", reinterpret_cast<int*>(&mSettings->line_algo), LineAlgorithm::DDA);
			ImGui::SameLine();
			ImGui::RadioButton("Bresenham", reinterpret_cast<int*>(&mSettings->line_algo), LineAlgorithm::Bresenham);
			ImGui::SameLine();
			ImGui::RadioButton("Wu", reinterpret_cast<int*>(&mSettings->line_algo), LineAlgorithm::Wu);

			ImGui::SeparatorText("Triangle algorithm");
			ImGui::RadioButton("Wireframe", reinterpret_cast<int*>(&mSettings->triangle_algo), TriangleAlgo::Wireframe);
			ImGui::SameLine();
			ImGui::RadioButton("Flat", reinterpret_cast<int*>(&mSettings->triangle_algo), TriangleAlgo::Flat);
			ImGui::SameLine();
			ImGui::RadioButton("Gouraud", reinterpret_cast<int*>(&mSettings->triangle_algo), TriangleAlgo::Gouraud);
			ImGui::End();
		}

		// Camera settings window
		if (mSettings && mSettings->open_windows.show_camera_window) {
			ImGui::Begin("Camera settings");
			ImGui::SliderFloat3("Position", &mSettings->camera.position[0], -100.0f, 100.0f);
			ImGui::SliderFloat3("Rotation", &mSettings->camera.rotation[0], -180.0f, 180.0f);
			ImGui::End();
		}
		ImGui::Render();

		draw_model();
	}

	void Display::draw_model() {
		if (mModel == nullptr || (mSettings->triangle_algo != TriangleAlgo::Wireframe)) {
			return;
		}
		for (int i = 0; i < mModel->nFaces(); i++) {
			std::vector<std::array<int, 3>> face = mModel->face(i);
			for (int j = 0; j < face.size(); j++) {
				glm::vec3 v0 = mModel->vert(face[j][0]);
				glm::vec3 v1 = mModel->vert(face[(j + 1) % 3][0]);
				if (mSettings->triangle_algo == TriangleAlgo::Wireframe) {
					int x0 = (v0.x + 1.) * mWinWidth / 2.;
					int y0 = (v0.y + 1.) * mWinHeight / 2.;
					int x1 = (v1.x + 1.) * mWinWidth / 2.;
					int y1 = (v1.y + 1.) * mWinHeight / 2.;
					draw_line(x0, y0, x1, y1, 0xFFFF0000);
				}
			}
		}
	}



	void Display::clear_color_buffer() {
		memset(mColorBuffer, 0x00000000, mWinWidth * mWinHeight * sizeof(uint32_t));
		/*
		__m256i colorSIMD = _mm256_set1_epi32(0xFF00FF00);
		int blockCount = static_cast<int>(mWinWidth * mWinHeight / 8);
		__m256i* blocks = (__m256i*) mColorBuffer;

		//SIMD as much as possible
		for (int block = 0; block < blockCount; ++block) {
			blocks[block] = colorSIMD;
		}

		//set any remaining pixels individually
		for (int pixel = blockCount * 8; pixel < mWinWidth * mWinHeight; ++pixel) {
			mColorBuffer[pixel] = 0xFFFF0000;
		}
		*/
	}

	void Display::draw_pixel(int x, int y, uint32_t color) {
		if (x >= 0 && x < mWinWidth && y >= 0 && y < mWinHeight) {
			mColorBuffer[(mWinWidth * y) + x] = color;
		}
	}
	void Display::draw_DDA(int x0, int y0, int x1, int y1, uint32_t color) {
		int delta_x = (x1 - x0);
		int delta_y = (y1 - y0);

		int longest_side_length = (abs(delta_x) >= abs(delta_y)) ? abs(delta_x) : abs(delta_y);

		float x_inc = delta_x / (float)longest_side_length;
		float y_inc = delta_y / (float)longest_side_length;

		float current_x = x0;
		float current_y = y0;

		for (int i = 0; i <= longest_side_length; i++) {
			draw_pixel(round(current_x), round(current_y), color);
			current_x += x_inc;
			current_y += y_inc;
		}
	}

	void Display::draw_bresenham(int x0, int y0, int x1, int y1, uint32_t color) {
		bool steep = false;
		if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
			std::swap(x0, y0);
			std::swap(x1, y1);
			steep = true;
		}
		if (x0 > x1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		int dx = x1 - x0;
		int dy = y1 - y0;
		int derror2 = std::abs(dy) * 2;
		int error2 = 0;
		int y = y0;
		for (int x = x0; x <= x1; x++) {
			if (steep) {
				draw_pixel(y, x, color);
			}
			else {
				draw_pixel(x, y, color);
			}
			error2 += derror2;
			if (error2 > dx) {
				y += (y1 > y0 ? 1 : -1);
				error2 -= dx * 2;
			}
		}
	}

	void Display::draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
		// Display::draw_DDA(x0, y0, x1, y1, color);
		Display::draw_bresenham(x0, y0, x1, y1, color);
	}


	void Display::draw_rect(int x, int y, int width, int height, uint32_t color) {
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				int current_x = x + i;
				int current_y = y + j;
				draw_pixel(current_x, current_y, color);
			}
		}
	}

	void Display::render() {
		SDL_RenderClear(mRenderer);
		SDL_UpdateTexture(mColorBufferTexture, nullptr, mColorBuffer, mWinWidth * sizeof(uint32_t));
		SDL_RenderCopy(mRenderer, mColorBufferTexture, nullptr, nullptr);
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(mRenderer);
		Display::clear_color_buffer();
	}

	void Display::destroy_window() {
		delete mColorBuffer;
		SDL_DestroyTexture(mColorBufferTexture);
		ImGui_ImplSDLRenderer2_Shutdown();
		SDL_DestroyRenderer(mRenderer);
		SDL_DestroyWindow(mWindow);
		SDL_Quit();
	}

	Display::~Display() {
		destroy_window();
	}
}