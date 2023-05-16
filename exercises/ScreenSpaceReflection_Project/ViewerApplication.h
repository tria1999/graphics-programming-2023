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
    void RenderIntoNormalsFBO();
    void RenderIntoUVCoordsFBO();
    void RenderIntoSpecularFBO();
    void RenderIntoColorFBO();
    void RenderIntoReflectionColorFBO();
    void RenderIntoBlurFBO();
    void Cleanup() override;

private:
    void InitializeModel();
    void InitializeCamera();

    void CreateWaterMaterial(std::shared_ptr<ShaderProgram> shaderProgram, ShaderUniformCollection::NameSet filteredUniforms);
    void CreateNormalsFramebufferMaterial();
    void CreateUVCoordsFramebufferMaterial();
    void CreateSpecularFramebufferMaterial();
    void CreateColorFramebufferMaterial();
    void CreateReflectionColorFramebufferMaterial();
    void CreateBlurFramebufferMaterial();
    void InitializeLights();
    void InitializeNormalsFBO();
    void InitializeUVCoordsFBO();
    void InitializeSpecularFBO();
    void InitializeColorFBO();
    void InitializeReflectionColorFBO();
    void InitializeBlurFBO();

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

    std::shared_ptr<FramebufferObject> m_normalsFBO;
    std::shared_ptr<FramebufferObject> m_UVCoordsFBO;
    std::shared_ptr<FramebufferObject> m_specularFBO;
    std::shared_ptr<FramebufferObject> m_colorFBO;
    std::shared_ptr<FramebufferObject> m_reflectionColorFBO;
    std::shared_ptr<FramebufferObject> m_blurFBO;
    //finally, render to screen

    std::shared_ptr<Texture2DObject> m_depthTexture;
    std::shared_ptr<Texture2DObject> m_normalTexture;
    std::shared_ptr<Texture2DObject> m_uvTexture;
    std::shared_ptr<Texture2DObject> m_specularTexture;
    std::shared_ptr<Texture2DObject> m_colorTexture;
    std::shared_ptr<Texture2DObject> m_reflectionColorTexture;
    std::shared_ptr<Texture2DObject> m_blurTexture;
    // will probably need the water texture too

    std::shared_ptr<Material> m_normalsMaterial;
    std::shared_ptr<Material> m_UVCoordsMaterial;
    std::shared_ptr<Material> m_specularMaterial;
    std::shared_ptr<Material> m_colorMaterial;
    std::shared_ptr<Material> m_reflectionColorMaterial;
    std::shared_ptr<Material> m_blurMaterial;
    std::shared_ptr<Material> m_waterMaterial;

    // Add light variables
    glm::vec3 m_ambientColor;
    glm::vec3 m_lightColor;
    float m_lightIntensity;
    glm::vec3 m_lightPosition;

    // Specular exponent debug
    float m_specularExponentGrass;

};
