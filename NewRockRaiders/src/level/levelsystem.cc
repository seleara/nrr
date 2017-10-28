#include <nrr/level/levelsystem.h>

#include <nrr/resource/shader/shader.h>
#include <nrr/math/uniformbuffer.h>

void LevelSystem::update() {

}

void LevelSystem::fixedUpdate() {

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