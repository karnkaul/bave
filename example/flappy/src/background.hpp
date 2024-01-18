#pragma once
#include <bave/graphics/sprite.hpp>
#include <src/config.hpp>

class Background {
  public:
	explicit Background(bave::NotNull<Config const*> config);

	void tick(bave::Seconds dt);
	void draw(bave::Shader& shader) const;

	bave::CustomShape quad;
	bave::Sprite cloud;

  private:
	struct CloudInstance {
		float speed{};
		glm::vec2 position{};
	};

	void create_quad();
	auto make_cloud(bool beyond_edge) const -> CloudInstance;
	void create_clouds();

	bave::NotNull<Config const*> m_config;
	bave::Rgba m_top{};
	bave::Rgba m_bottom{};

	std::vector<CloudInstance> m_cloud_instances{};
};
