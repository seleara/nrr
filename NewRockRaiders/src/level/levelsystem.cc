#include <nrr/level/levelsystem.h>

#include <nrr/resource/shader/shader.h>
#include <nrr/math/uniformbuffer.h>
#include <nrr/math/ray.h>
#include <nrr/window/window.h>
#include <nrr/math/camera.h>
#include <nrr/math/transform.h>
#include <nrr/model/modelrenderingsystem.h>

#include <imgui/imgui.h>

void LevelSystem::init() {
}

void LevelSystem::update() {
	auto mpos = Window::main().mousePosition();
	one<Level>([&](Level &l) {
		auto ct = CameraComponent::main.entity().get<TransformComponent>();
		auto nearPoint = ct->position;
		//Ray ray(ct->position, farPoint);
		float mx = mpos.x / (Window::main().size().x * 0.5f) - 1.0f;
		float my = mpos.y / (Window::main().size().y * 0.5f) - 1.0f;
		auto mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
		const auto &proj = mvp->projection;
		const auto &view = mvp->view;

		auto ray_Px = mx / proj[0][0];
		auto ray_Py = -my / proj[1][1];
		auto ray_P = glm::vec4(ray_Px, ray_Py, -1.0f, 1.0f);

		auto invV = glm::inverse(view);
		auto dir = glm::vec3(glm::normalize(invV * ray_P - glm::vec4(ct->position, 1.0f)));

		auto far = ct->position + dir * 10000.0f;
		Ray ray(ct->position, far);
		glm::vec3 point;

		bool debugWindowOpen = true;
		ImGui::Begin("Ray", &debugWindowOpen);
		ImGui::BeginChild("ray", ImVec2(0, 0), false);
		ImGui::Text("MX: %f  MY: %f", mx, my);
		ImGui::Text("FX: %f  FY: %f  FZ: %f", far.x, far.y, far.z);

		bool found = false;
		for (int y = 0; y < l.size.y; ++y) {
			for (int x = 0; x < l.size.x; ++x) {
				auto &t = *l.tile(x, y);
				auto index = l.tileVertexIndex(x, y);
				for (int i = 0; i < 2; ++i) {
					const auto &p1 = l.vertices[index + i * 3 + 0].position;
					const auto &p2 = l.vertices[index + i * 3 + 1].position;
					const auto &p3 = l.vertices[index + i * 3 + 2].position;

					if (ray.intersects({ p1, p2, p3 }, point)) {
						//std::cout << "Intersection at tile " << x << ", " << y << ".\n";
						t.highlightColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
						t.highlighted = true;
						t.dirty = true;
						if (!found) {
							ImGui::Text("TX: %f  TY: %f", x, y);
							ImGui::Text("IX: %f  IY: %f  IZ: %f", point.x, point.y, point.z);

							found = true;
						}
						break;
					} else {
						if (t.highlightColor != glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) {
							t.highlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
							t.highlighted = false;
							t.dirty = true;
						}
					}
				}
			}
		}

		if (!found) {
			ImGui::Text("No intersection.");
		}

		ImGui::EndChild();
		ImGui::End();
	});
}

void LevelSystem::fixedUpdate() {
	forEach<Level>([&](Level &l) {
		for (int y = 0; y < l.size.y; ++y) {
			for (int x = 0; x < l.size.x; ++x) {
				if (l.updateTile(x, y)) {
					l.uploadTile(x, y);
				}
			}
		}
	});
}

void LevelSystem::render() {
	Shader shader;
	shader.loadCache("level");
	shader.bind();
	glm::vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
	glUniform4fv(0, 1, &white[0]);
	glUniform1i(3, 0);
	auto mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
	mvp->model = glm::mat4();
	mvp.update();
	// We'll only ever have one single level instantiated at a time so this is probably overkill
	forEach<Level>([&](Level &l) {
		glBindVertexArray(l.vao);
		l.atlas.bind();
		l.buffer.draw(Primitives::Triangles, 0, l.tiles.size() * 6);
	});
	glBindVertexArray(0);
}