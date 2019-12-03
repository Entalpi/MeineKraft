#include "meinekraft.hpp"

#include <chrono>

#include "rendering/renderer.hpp"
#include "../include/imgui/imgui_impl_sdl.h"
#include "rendering/camera.hpp"
#include "nodes/model.hpp"
#include "util/filesystem.hpp"
#include "scene/world.hpp"
#include "rendering/debug_opengl.hpp"
#include "nodes/skybox.hpp"
#include "scene/world.hpp"
#include "rendering/graphicsbatch.hpp"
#include "util/config.hpp"

void imgui_styling() {
  ImGuiStyle* style = &ImGui::GetStyle();

  style->WindowPadding = ImVec2(15, 15);
  style->WindowRounding = 5.0f;
  style->FramePadding = ImVec2(5, 5);
  style->FrameRounding = 4.0f;
  style->ItemSpacing = ImVec2(12, 8);
  style->ItemInnerSpacing = ImVec2(8, 6);
  style->IndentSpacing = 25.0f;
  style->ScrollbarSize = 15.0f;
  style->ScrollbarRounding = 9.0f;
  style->GrabMinSize = 5.0f;
  style->GrabRounding = 3.0f;

  style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
  style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
  style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.80f);
  style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
  style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
  style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
  style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
  style->Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.19f, 0.22f, 1.00f);
  style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.19f, 0.25f, 1.00f);
  style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
  style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
  style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
  style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
  style->Colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 1.0f, 0.40f, 0.31f);
  style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
  style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
  style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
  style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
  style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
  style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
  style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
  style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.88f, 0.63f);
  style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 1.00f, 1.0f, 1.00f);
  style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
  style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 1.00f, 1.0f, 1.00f);
  style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.55f, 0.55f, 0.55f, 0.43f);
  style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
  // TODO: Change PlotLines color depending on the value (set thresholds for the different colors)
}

MeineKraft::MeineKraft() {
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VERSION);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
  auto window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_MOUSE_CAPTURE;
  window = SDL_CreateWindow("MeineKraft", 0, 0, HD.width, HD.height, window_flags);
  SDL_GLContext context = SDL_GL_CreateContext(window);
  if (!context) { Log::error(std::string(SDL_GetError())); }
  SDL_GL_SetSwapInterval(0); // Disables vsync

  glewExperimental = (GLboolean) true;
  glewInit();

  OpenGLContextInfo gl_context_info(4, OPENGL_MINOR_VERSION);

  atexit(IMG_Quit);
  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG); 
  
  ImGui_ImplSdlGL3_Init(window);

  renderer = new Renderer(HD);
  renderer->update_projection_matrix(70.0f, HD);

  imgui_styling();
}

void MeineKraft::init() {
  bool success = false;
  const auto config = Config::load_config(success);
  if (success) {
    const std::string path = config["scene"]["path"].get<std::string>();
    const std::string name = config["scene"]["name"].get<std::string>();
    // renderer->scene = new Scene("/home/alexander/Desktop/Meinekraft/BoomBox/", "BoomBox.gltf");
    renderer->scene = new Scene(Filesystem::home + path, name);
    // renderer->scene = new Scene("/home/alexander/Desktop/Meinekraft/MetalRoughSpheres/", "MetalRoughSpheres.gltf");
    // renderer->scene->load_models_from("/home/alexander/Desktop/Meinekraft/Suzanne/", "Suzanne.gltf");
    // renderer->scene->load_models_from("/home/alexander/Desktop/Meinekraft/MetalRoughSpheres/", "MetalRoughSpheres.gltf");
    // renderer->scene->load_models_from("/home/alexander/Desktop/Meinekraft/BoomBox/", "BoomBox.gltf");
  } else {
    // TODO: Load default scene or smt
    Log::error("Failed to load config.json.");
  }

  renderer->init();
}

MeineKraft::~MeineKraft() {
  if (renderer) { delete renderer; }
  ImGui_ImplSdlGL3_Shutdown();
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void MeineKraft::mainloop() {
  World world;

  bool toggle_mouse_capture = true;
  bool done = false;
  auto last_tick = std::chrono::high_resolution_clock::now();
  auto current_tick = last_tick;
  int64_t delta = 0;

  /// Delta values
  const int num_deltas = 100;
  float deltas[num_deltas];

  while (!done) {
      current_tick = std::chrono::high_resolution_clock::now();
      delta = std::chrono::duration_cast<std::chrono::milliseconds>(current_tick - last_tick).count();
      last_tick = current_tick;

      /// Window handling
      static bool throttle_rendering = false;

      /// Process input
      SDL_Event event{};
      while (SDL_PollEvent(&event) != 0) {
        ImGui_ImplSdlGL3_ProcessEvent(&event);
        switch (event.type) {
        case SDL_MOUSEMOTION:
          if (toggle_mouse_capture) { break; }
          renderer->scene->camera->pitch += event.motion.yrel;
          renderer->scene->camera->yaw += event.motion.xrel;
          renderer->scene->camera->direction = renderer->scene->camera->recalculate_direction();
          break;

        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
					case SDLK_1:
						renderer->state.camera_selection = 0;
						break;
					case SDLK_2:
						renderer->state.camera_selection = 1;
						break;
					case SDLK_3:
						renderer->state.camera_selection = 2;
						break;
          case SDLK_w:
            renderer->scene->camera->move_forward(true);
            break;
          case SDLK_a:
            renderer->scene->camera->move_left(true);
            break;
          case SDLK_s:
            renderer->scene->camera->move_backward(true);
            break;
          case SDLK_d:
            renderer->scene->camera->move_right(true);
            break;
          case SDLK_q:
            renderer->scene->camera->move_down(true);
            break;
          case SDLK_e:
            renderer->scene->camera->move_up(true);
            break;
          case SDLK_TAB:
            toggle_mouse_capture = !toggle_mouse_capture;
            break;
          case SDLK_ESCAPE:
            done = true;
            break;
          }
          break;

        case SDL_KEYUP:
          switch (event.key.keysym.sym) {
          case SDLK_w:
            renderer->scene->camera->move_forward(false);
            break;
          case SDLK_a:
            renderer->scene->camera->move_left(false);
            break;
          case SDLK_s:
            renderer->scene->camera->move_backward(false);
            break;
          case SDLK_d:
            renderer->scene->camera->move_right(false);
            break;
          case SDLK_q:
            renderer->scene->camera->move_down(false);
            break;
          case SDLK_e:
            renderer->scene->camera->move_up(false);
          }

        case SDL_WINDOWEVENT:
          switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              Resolution screen;
              SDL_GetWindowSize(window, &screen.width, &screen.height);
              renderer->update_projection_matrix(70.0f, screen);
              break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
              throttle_rendering = false;
              break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
              // throttle_rendering = true;
              break;
          }
          break;
        case SDL_QUIT:
          done = true;
          break;
      }
    }
    renderer->scene->camera->position = renderer->scene->camera->update(delta);

    /// Run all actions
    ActionSystem::instance().execute_actions(renderer->state.frame, delta);

    /// Let the game do its thing
    world.tick();

    /// Render the world
    if (!throttle_rendering) {      
      renderer->render(delta);
    }

    /// ImGui - Debug instruments
    if (!throttle_rendering) {
      pass_started("ImGui");

      ImGui_ImplSdlGL3_NewFrame(window);
      auto io = ImGui::GetIO();
      ImGui::Begin("MeineKraft");

      ImGui::PushItemWidth(90.0f);

      if (ImGui::CollapsingHeader("Render System", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Frame: %lu", renderer->state.frame);
        ImGui::Text("Entities: %lu", renderer->state.entities);
        ImGui::Text("Average %lu ms / frame (%.1f FPS)", delta, io.Framerate);

        static size_t i = -1; i = (i + 1) % num_deltas;
        deltas[i] = float(delta);
        ImGui::PlotLines("", deltas, num_deltas, 0, "ms / frame", 0.0f, 50.0f, ImVec2(ImGui::GetWindowWidth(), 100));

        ImGui::InputFloat("Shadow bias", &renderer->state.shadow_bias);
        ImGui::Checkbox("Normal mapping", &renderer->state.normalmapping);
        ImGui::Checkbox("Conservative raster.", &renderer->state.conservative_rasterization);

        if (ImGui::Button("Voxelize")) {
          renderer->state.voxelize = true;
        }
        ImGui::Checkbox("Always voxelize", &renderer->state.always_voxelize);

        ImGui::Checkbox("Direct", &renderer->state.direct_lighting);
        ImGui::SameLine();
        ImGui::Checkbox("Indirect", &renderer->state.indirect_lighting);


        ImGui::Checkbox("Diffuse", &renderer->state.diffuse_lighting);
        ImGui::SameLine();
        ImGui::Checkbox("Specular", &renderer->state.specular_lighting);
        ImGui::SameLine();
        ImGui::Checkbox("Ambient", &renderer->state.ambient_lighting);

        ImGui::SliderInt("# diffuse cones", &renderer->state.num_diffuse_cones, 1, 12);

        ImGui::InputFloat("Roughness", &renderer->state.roughness);
        ImGui::InputFloat("Metallic", &renderer->state.metallic);
        ImGui::InputFloat("Roughness aperature (deg.)", &renderer->state.roughness_aperature);
        ImGui::InputFloat("Metallic aperature (deg.)", &renderer->state.metallic_aperature);

        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
          ImGui::PushItemWidth(200.0f);
          ImGui::InputFloat3("Position##camera", &renderer->scene->camera->position.x);
          ImGui::InputFloat3("Direction##camera", &renderer->scene->camera->direction.x);
          ImGui::PopItemWidth();
          // ImGui::InputFloat("FoV", camera->FoV); // TODO: Camera adjustable FoV
          if (ImGui::Button("Reset camera")) {
            renderer->scene->reset_camera();
          }
        }

        // Directional light
        const std::string directional_light_title = "Directional light";
        if (ImGui::CollapsingHeader(directional_light_title.c_str())) {
          ImGui::InputFloat3("Direction##directional_light", &renderer->directional_light.direction.x);
        }

        // Point lights
        const std::string pointlights_title = "Point lights (" + std::to_string(renderer->pointlights.size()) + ")";
        if (ImGui::CollapsingHeader(pointlights_title.c_str())) {
          for (size_t i = 0; i < renderer->pointlights.size(); i++) {
            ImGui::PushID(&renderer->pointlights[i]);
            const std::string str = std::to_string(i);
            if (ImGui::CollapsingHeader(str.c_str())) {
              ImGui::InputFloat3("Position", &renderer->pointlights[i].position.x);
              ImGui::InputFloat3("Intensity", &renderer->pointlights[i].intensity.x);
            }
            ImGui::PopID();
          }
        }

        // Graphics batches
        const std::string graphicsbatches_title = "Graphics batches (" + std::to_string(renderer->graphics_batches.size()) + ")";
        if (ImGui::CollapsingHeader(graphicsbatches_title.c_str())) {
          for (size_t batch_num = 0; batch_num < renderer->graphics_batches.size(); batch_num++) {
            const auto& batch = renderer->graphics_batches[batch_num];
            const std::string batch_title = "Batch #" + std::to_string(batch_num + 1) + " (" + std::to_string(batch.entity_ids.size()) + ")";

            if (ImGui::CollapsingHeader(batch_title.c_str())) {
              const std::string member_title = "Members##" + batch_title;
              if (ImGui::CollapsingHeader(member_title.c_str())) {
                for (const auto& id : batch.entity_ids) {
                  const std::string* name = NameSystem::instance().get_name_from_entity_referenced(id);
                  ImGui::Text("Entity id: %lu, Name: %s", id, name->c_str());
                  TransformComponent* transform = TransformSystem::instance().lookup_referenced(id);
                  ImGui::PushID(transform);
                  ImGui::InputFloat3("Position", &transform->position.x);
                  ImGui::InputFloat3("Rotation", &transform->rotation.x);
                  ImGui::InputFloat("Scale", &transform->scale);
                  ImGui::PopID();
                }
              }
            }
          }
        }

        ImGui::PopItemWidth();

        ImGui::End();
        ImGui::Render();
        pass_ended();
      }

    }
    SDL_GL_SwapWindow(window);
  }
  Config::save_scene(renderer->scene);
}
