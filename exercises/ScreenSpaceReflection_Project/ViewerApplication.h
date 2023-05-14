#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/shader/Material.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/utils/DearImGui.h>
#include <ituGL/texture/FramebufferObject.h>

class Texture2DObject;

class ViewerApplication : public Application
{
public:
    ViewerApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeModel();
    void InitializeCamera();
    void CreateWaterMaterial(std::shared_ptr<ShaderProgram> shaderProgram, ShaderUniformCollection::NameSet filteredUniforms);
    void CreateFramebufferMaterial();
    void InitializeLights();
    void InitializeFrameBufferObject();

    void UpdateCamera();

    void RenderGUI();


private:
    // Helper object for debug GUI
    DearImGui m_imGui;

    // Mouse position for camera controller
    glm::vec2 m_mousePosition;

    // Camera controller parameters
    Camera m_camera;
    glm::vec3 m_cameraPosition;
    float m_cameraTranslationSpeed;
    float m_cameraRotationSpeed;
    bool m_cameraEnabled;
    bool m_cameraEnablePressed;

    // Loaded model
    Model m_model;
    Model m_waterSurface;

    std::shared_ptr<FramebufferObject> m_fbo;

    std::shared_ptr<Texture2DObject> m_depthTexture;
    std::shared_ptr<Texture2DObject> m_normalTexture;

    std::shared_ptr<Material> m_waterMaterial;
    std::shared_ptr<Material> m_framebufferMaterial;

    // Add light variables
    glm::vec3 m_ambientColor;
    glm::vec3 m_lightColor;
    float m_lightIntensity;
    glm::vec3 m_lightPosition;

    // Specular exponent debug
    float m_specularExponentGrass;

};
