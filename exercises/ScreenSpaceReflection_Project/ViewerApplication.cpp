#include "ViewerApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/texture/Texture2DObject.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>
#include <iostream>
#include <ituGL/renderer/GBufferRenderPass.h>

ViewerApplication::ViewerApplication()
    : Application(1024, 1024, "Viewer demo")
    , m_cameraPosition(0, 30, 30)
    , m_cameraTranslationSpeed(20.0f)
    , m_cameraRotationSpeed(0.5f)
    , m_cameraEnabled(false)
    , m_cameraEnablePressed(false)
    , m_mousePosition(GetMainWindow().GetMousePosition(true))
    , m_ambientColor(0.0f)
    , m_lightColor(0.0f)
    , m_lightIntensity(0.0f)
    , m_lightPosition(0.0f)
    , m_specularExponentGrass(100.0f)
{
}

void ViewerApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    InitializeNormalsFBO();
    InitializeUVCoordsFBO();
    InitializeSpecularFBO();
    InitializeColorFBO();
    InitializeReflectionColorFBO();
    InitializeBlurFBO();
    InitializeModel();
    InitializeCamera();
    InitializeLights();

    DeviceGL& device = GetDevice();
    device.EnableFeature(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    device.SetVSyncEnabled(true);
}

void ViewerApplication::Update()
{
    Application::Update();

    // Update camera controller
    UpdateCamera();

    // Update specular exponent for grass material
    m_model.GetMaterial(1).SetUniformValue("SpecularExponent", m_specularExponentGrass);
    // Pass the time for the wave animation in float form
    m_waterSurface.GetMaterial(0).SetUniformValue("time", (float)glfwGetTime());
}

void ViewerApplication::Render()
{
    Application::Render();

    // Clear color and depth
    //GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    RenderIntoNormalsFBO();
    m_UVCoordsMaterial->SetUniformValue("DepthTexture", m_depthTexture);
    m_UVCoordsMaterial->SetUniformValue("NormalTexture", m_normalTexture);
    //Note: UVCoords are in world space?
    RenderIntoUVCoordsFBO();
    RenderIntoSpecularFBO();
    RenderIntoColorFBO();
    m_reflectionColorMaterial->SetUniformValue("UVTexture", m_uvTexture);
    m_reflectionColorMaterial->SetUniformValue("ColorTexture", m_colorTexture);
    RenderIntoReflectionColorFBO();
    m_blurMaterial->SetUniformValue("ColorTexture", m_colorTexture);
    RenderIntoBlurFBO();
    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);
    m_model.Draw();    


    //Update the uniforms each frame
    m_waterSurface.GetMaterial(0).SetUniformValue("DepthTexture",m_depthTexture);
    m_waterSurface.GetMaterial(0).SetUniformValue("NormalTexture", m_normalTexture);
    m_waterSurface.Draw();
    
    // Render the debug user interface
    RenderGUI();
}

// I used different functions because there were errors if I used 1 functions for each case,
// passing each FBO and material as arguments
void ViewerApplication::RenderIntoNormalsFBO()
{
    // FIRST PART: RENDERING THE FRAMEBUFFER
    m_normalsFBO->Bind();

    // Clear color and depth. Yes, you also need to clear the textures. Maybe you were missing this part?
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Draw the mill model with a different material that only outputs the normals

    // Set up the normals material before rendering
    m_normalsMaterial->Use();
    // Draw all the submeshes, one by one, all with the same material
    for (unsigned int submeshIndex = 0; submeshIndex < m_model.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_model.GetMesh().DrawSubmesh(submeshIndex);
    }
    
    for (unsigned int submeshIndex = 0; submeshIndex < m_waterSurface.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_waterSurface.GetMesh().DrawSubmesh(submeshIndex);
    }
    m_normalsFBO->Unbind();
}

void ViewerApplication::RenderIntoUVCoordsFBO()
{
    //m_UVCoordsFBO->Bind();

    // Clear color and depth. Yes, you also need to clear the textures. Maybe you were missing this part?
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Draw the mill model with a different material that only outputs the normals

    // Set up the normals material before rendering
    m_UVCoordsMaterial->Use();
    // Draw all the submeshes, one by one, all with the same material
    for (unsigned int submeshIndex = 0; submeshIndex < m_model.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_model.GetMesh().DrawSubmesh(submeshIndex);
    }

    for (unsigned int submeshIndex = 0; submeshIndex < m_waterSurface.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_waterSurface.GetMesh().DrawSubmesh(submeshIndex);
    }
    //m_UVCoordsFBO->Unbind();
}

void ViewerApplication::RenderIntoSpecularFBO()
{
    m_specularFBO->Bind();

    // Clear color and depth. Yes, you also need to clear the textures. Maybe you were missing this part?
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Draw the mill model with a different material that only outputs the normals

    // Set up the normals material before rendering
    m_specularMaterial->Use();
    // Draw all the submeshes, one by one, all with the same material
    for (unsigned int submeshIndex = 0; submeshIndex < m_model.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_model.GetMesh().DrawSubmesh(submeshIndex);
    }
    
    for (unsigned int submeshIndex = 0; submeshIndex < m_waterSurface.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_waterSurface.GetMesh().DrawSubmesh(submeshIndex);
    }
    m_specularFBO->Unbind();
}

void ViewerApplication::RenderIntoColorFBO()
{
    m_colorFBO->Bind();

    // Clear color and depth. Yes, you also need to clear the textures. Maybe you were missing this part?
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Draw the mill model with a different material that only outputs the normals

    // Set up the normals material before rendering
    m_colorMaterial->Use();
    // Draw all the submeshes, one by one, all with the same material
    for (unsigned int submeshIndex = 0; submeshIndex < m_model.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_model.GetMesh().DrawSubmesh(submeshIndex);
    }

    for (unsigned int submeshIndex = 0; submeshIndex < m_waterSurface.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_waterSurface.GetMesh().DrawSubmesh(submeshIndex);
    }
    m_colorFBO->Unbind();
}

void ViewerApplication::RenderIntoReflectionColorFBO()
{
    m_reflectionColorFBO->Bind();

    // Clear color and depth. Yes, you also need to clear the textures. Maybe you were missing this part?
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Draw the mill model with a different material that only outputs the normals

    // Set up the normals material before rendering
    m_reflectionColorMaterial->Use();
    // Draw all the submeshes, one by one, all with the same material
    for (unsigned int submeshIndex = 0; submeshIndex < m_model.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_model.GetMesh().DrawSubmesh(submeshIndex);
    }

    for (unsigned int submeshIndex = 0; submeshIndex < m_waterSurface.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_waterSurface.GetMesh().DrawSubmesh(submeshIndex);
    }
    m_reflectionColorFBO->Unbind();
}

void ViewerApplication::RenderIntoBlurFBO()
{
    // FIRST PART: RENDERING THE FRAMEBUFFER
    m_blurFBO->Bind();

    // Clear color and depth. Yes, you also need to clear the textures. Maybe you were missing this part?
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Draw the mill model with a different material that only outputs the normals

    // Set up the normals material before rendering
    m_blurMaterial->Use();
    // Draw all the submeshes, one by one, all with the same material
    for (unsigned int submeshIndex = 0; submeshIndex < m_model.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_model.GetMesh().DrawSubmesh(submeshIndex);
    }

    for (unsigned int submeshIndex = 0; submeshIndex < m_waterSurface.GetMesh().GetSubmeshCount(); ++submeshIndex)
    {
        // Draw the submesh
        m_waterSurface.GetMesh().DrawSubmesh(submeshIndex);
    }
    m_blurFBO->Unbind();
}


void ViewerApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void ViewerApplication::InitializeModel()
{
    // Load and build shader
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/blinn-phong.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/blinn-phong.frag");
    //Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/framebuffer.vert");
    //Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/framebuffer.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    // Filter out uniforms that are not material properties
    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");
    filteredUniforms.insert("AmbientColor");
    filteredUniforms.insert("LightColor");

    // Create reference material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgram, filteredUniforms);
    material->SetUniformValue("Color", glm::vec4(1.0f));
    material->SetUniformValue("AmbientReflection", 1.0f);
    material->SetUniformValue("DiffuseReflection", 1.0f);
    material->SetUniformValue("SpecularReflection", 1.0f);
    material->SetUniformValue("SpecularExponent", 100.0f);

    // Setup function
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location ambientColorLocation = shaderProgram->GetUniformLocation("AmbientColor");
    ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
    ShaderProgram::Location lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
    ShaderProgram::Location cameraPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");
    material->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)) );//* glm::rotate(glm::radians(45.0f),glm::vec3(1,0,0))
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());

            // Set camera and light uniforms
            shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
            shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
            shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
            shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
        });

    // Configure loader
    ModelLoader loader(material);
    loader.SetCreateMaterials(true);
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");

    // Load model
    

    // Load and set textures
    Texture2DLoader textureLoader(TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    textureLoader.SetFlipVertical(true);
   
    m_model = loader.Load("models/mill/Mill.obj");
    m_model.GetMaterial(0).SetUniformValue("ColorTexture", textureLoader.LoadShared("models/mill/Ground_shadow.jpg"));
    m_model.GetMaterial(1).SetUniformValue("ColorTexture", textureLoader.LoadShared("models/mill/Ground_color.jpg"));
    m_model.GetMaterial(2).SetUniformValue("ColorTexture", textureLoader.LoadShared("models/mill/MillCat_color.jpg"));
    
    CreateWaterMaterial(shaderProgram, filteredUniforms);
    ModelLoader waterLoader(m_waterMaterial);

    waterLoader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "position");
    waterLoader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
    waterLoader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "texCoord");

    waterLoader.SetMaterialProperty(ModelLoader::MaterialProperty::NormalTexture, "NormalTexture");

    m_waterSurface = waterLoader.Load("models/water/water2.obj");
    std::shared_ptr<Texture2DObject> waterTexture = textureLoader.LoadShared("models/water/water.png");
    /*
    waterTexture->Bind();
    waterTexture->SetParameter(TextureObject::ParameterEnum::WrapT, GL_REPEAT);
    waterTexture->SetParameter(TextureObject::ParameterEnum::WrapS, GL_REPEAT);
    waterTexture->Unbind();
    */
    m_waterSurface.GetMaterial(0).SetUniformValue("ColorTexture", waterTexture);
    //m_waterSurface.GetMaterial(0).SetUniformValue("DepthTexture", m_depthTexture);
    //m_waterSurface.GetMaterial(0).SetUniformValue("NormalTexture", m_normalTexture);
}

void ViewerApplication::InitializeCamera()
{
    // Set view matrix, from the camera position looking to the origin
    m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

    // Set perspective matrix
    float aspectRatio = GetMainWindow().GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);
}

//This is the SSR material of the project
void ViewerApplication::CreateWaterMaterial(std::shared_ptr<ShaderProgram> shaderProgram2, ShaderUniformCollection::NameSet filteredUniforms)
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/SSR_let.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/SSR_let.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    filteredUniforms.insert("ProjMatrix");
    filteredUniforms.insert("InvProjMatrix");
    filteredUniforms.insert("InvViewMatrix");

    m_waterMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);
    m_waterMaterial->SetUniformValue("Color", glm::vec4(1.0f));
    m_waterMaterial->SetUniformValue("AmbientReflection", 1.0f);
    m_waterMaterial->SetUniformValue("DiffuseReflection", 1.0f);
    m_waterMaterial->SetUniformValue("SpecularReflection", 1.0f);
    m_waterMaterial->SetUniformValue("SpecularExponent", 100.0f);
    m_waterMaterial->SetUniformValue("time", 0.0f);

    // Setup function
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location ambientColorLocation = shaderProgram->GetUniformLocation("AmbientColor");
    ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
    ShaderProgram::Location lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
    ShaderProgram::Location cameraPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");

    ShaderProgram::Location projMatrixLocation = shaderProgram->GetUniformLocation("ProjMatrix");
    ShaderProgram::Location invProjMatrixLocation = shaderProgram->GetUniformLocation("InvProjMatrix");
    ShaderProgram::Location invViewMatrixLocation = shaderProgram->GetUniformLocation("InvViewMatrix");
    //ShaderProgram::Location depthTextureLocation = shaderProgram->GetUniformLocation("DepthTexture");

    m_waterMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));//glm::translate(glm::vec3(0.f,2.f,0.f))*
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
            shaderProgram.SetUniform(projMatrixLocation, m_camera.GetProjectionMatrix());
            shaderProgram.SetUniform(invProjMatrixLocation, glm::inverse(m_camera.GetProjectionMatrix()));
            // Set camera and light uniforms
            shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
            shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
            shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
            shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
            shaderProgram.SetUniform(invViewMatrixLocation, glm::inverse(m_camera.GetViewMatrix()));
            //shaderProgram.SetUniform(depthTextureLocation, *m_depthTexture);
        });
}

void ViewerApplication::CreateNormalsFramebufferMaterial()
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/normal.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/normal.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("ProjMatrix");
    filteredUniforms.insert("InvProjMatrix");
    //filteredUniforms.insert("ViewMatrix");
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");
    m_normalsMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);

    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location projMatrixLocation = shaderProgram->GetUniformLocation("ProjMatrix");
    ShaderProgram::Location invProjMatrixLocation = shaderProgram->GetUniformLocation("InvProjMatrix");
    //ShaderProgram::Location viewMatrixLocation = shaderProgram->GetUniformLocation("ViewMatrix");
    
    m_normalsMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            //shaderProgram.SetUniform(viewMatrixLocation, m_camera.GetViewMatrix());
            shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
            shaderProgram.SetUniform(projMatrixLocation, m_camera.GetProjectionMatrix());
            shaderProgram.SetUniform(invProjMatrixLocation, glm::inverse(m_camera.GetProjectionMatrix()));
        });
}

void ViewerApplication::CreateUVCoordsFramebufferMaterial()
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/SSR_uv.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/SSR_uv.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("ProjMatrix");
    filteredUniforms.insert("InvProjMatrix");
    filteredUniforms.insert("InvViewMatrix");

    m_UVCoordsMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);

    // Setup function
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");

    ShaderProgram::Location cameraPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");

    ShaderProgram::Location projMatrixLocation = shaderProgram->GetUniformLocation("ProjMatrix");
    ShaderProgram::Location invProjMatrixLocation = shaderProgram->GetUniformLocation("InvProjMatrix");
    ShaderProgram::Location invViewMatrixLocation = shaderProgram->GetUniformLocation("InvViewMatrix");


    m_UVCoordsMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
            shaderProgram.SetUniform(projMatrixLocation, m_camera.GetProjectionMatrix());
            shaderProgram.SetUniform(invProjMatrixLocation, glm::inverse(m_camera.GetProjectionMatrix()));
            // Set camera and light uniforms
            shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
            shaderProgram.SetUniform(invViewMatrixLocation, glm::inverse(m_camera.GetViewMatrix()));
        });
}

void ViewerApplication::CreateSpecularFramebufferMaterial()
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/specular.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/specular.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    // Filter out uniforms that are not material properties
    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");
    filteredUniforms.insert("LightColor");

    m_specularMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);

    m_specularMaterial->SetUniformValue("SpecularReflection", 1.0f);
    m_specularMaterial->SetUniformValue("SpecularExponent", 100.0f);
    // Setup function
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
    ShaderProgram::Location cameraPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");

    m_specularMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
    {
        shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));
        shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
        // Set camera and light uniforms
        shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
        shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
    });
}

void ViewerApplication::CreateColorFramebufferMaterial()
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/blinn-phong.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/blinn-phong.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    // Filter out uniforms that are not material properties
    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");
    filteredUniforms.insert("AmbientColor");
    filteredUniforms.insert("LightColor");

    // Create reference material
    m_colorMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);
    m_colorMaterial->SetUniformValue("Color", glm::vec4(1.0f));
    m_colorMaterial->SetUniformValue("AmbientReflection", 1.0f);
    m_colorMaterial->SetUniformValue("DiffuseReflection", 1.0f);
    m_colorMaterial->SetUniformValue("SpecularReflection", 1.0f);
    m_colorMaterial->SetUniformValue("SpecularExponent", 100.0f);

    // Setup function
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location ambientColorLocation = shaderProgram->GetUniformLocation("AmbientColor");
    ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
    ShaderProgram::Location lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
    ShaderProgram::Location cameraPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");
    m_colorMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));//* glm::rotate(glm::radians(45.0f),glm::vec3(1,0,0))
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());

            // Set camera and light uniforms
            shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
            shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
            shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
            shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
        });
}

void ViewerApplication::CreateReflectionColorFramebufferMaterial()
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/reflectionColor.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/reflectionColor.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    // Filter out uniforms that are not material properties
    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");

    m_reflectionColorMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);

    // Setup function
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");

    m_reflectionColorMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
        });
}

void ViewerApplication::CreateBlurFramebufferMaterial()
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/reflectionColor.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/reflectionColor.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    // Filter out uniforms that are not material properties
    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");

    m_blurMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);
    m_blurMaterial->SetUniformValue("Parameters", glm::vec2(3.0f,2.0f));
    // Setup function
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");

    m_blurMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
        });
}

void ViewerApplication::InitializeLights()
{
    // Initialize light variables
    m_ambientColor = glm::vec3(0.25f);
    m_lightColor = glm::vec3(1.0f);
    m_lightIntensity = 1.0f;
    m_lightPosition = glm::vec3(-10.0f, 20.0f, 10.0f);
}


void ViewerApplication::InitializeNormalsFBO()
{
       
    m_normalsFBO = std::make_shared<FramebufferObject>();
    m_normalsFBO->Bind();

    m_depthTexture = std::make_shared<Texture2DObject>();
    m_depthTexture->Bind();
    m_depthTexture->SetImage(0, 1024, 1024,TextureObject::FormatDepth, TextureObject::InternalFormatDepth);
    m_depthTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_depthTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);
    m_depthTexture->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
    m_depthTexture->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);

    m_depthTexture->Unbind();

    m_normalsFBO->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Depth, *m_depthTexture);

    m_normalTexture = std::make_shared<Texture2DObject>();
    m_normalTexture->Bind();
    m_normalTexture->SetImage(0, 1024, 1024, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    m_normalTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_normalTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);
    m_normalTexture->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
    m_normalTexture->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
    
    m_normalTexture->Unbind();

    m_normalsFBO->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Color0, *m_normalTexture);

    m_normalsFBO->SetDrawBuffers(std::array<FramebufferObject::Attachment, 1>({ FramebufferObject::Attachment::Color0 }));
  
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete." << std::endl;
    }
    
    m_normalsFBO->Unbind();

    CreateNormalsFramebufferMaterial();
}
void ViewerApplication::InitializeUVCoordsFBO()
{

    m_UVCoordsFBO = std::make_shared<FramebufferObject>();
    m_UVCoordsFBO->Bind();

    m_uvTexture = std::make_shared<Texture2DObject>();
    m_uvTexture->Bind();
    m_uvTexture->SetImage(0, 1024, 1024, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    m_uvTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_uvTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);
    m_uvTexture->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
    m_uvTexture->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
    m_uvTexture->Unbind();

    m_UVCoordsFBO->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Color0, *m_uvTexture);
    m_UVCoordsFBO->SetDrawBuffers(std::array<FramebufferObject::Attachment, 1>({ FramebufferObject::Attachment::Color0 }));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete." << std::endl;
    }

    m_UVCoordsFBO->Unbind();

    CreateUVCoordsFramebufferMaterial();
}
void ViewerApplication::InitializeSpecularFBO()
{

    m_specularFBO = std::make_shared<FramebufferObject>();
    m_specularFBO->Bind();

    m_specularTexture = std::make_shared<Texture2DObject>();
    m_specularTexture->Bind();
    m_specularTexture->SetImage(0, 1024, 1024, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    m_specularTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_specularTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);
    m_specularTexture->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
    m_specularTexture->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
    m_specularTexture->Unbind();

    m_specularFBO->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Color0, *m_specularTexture);
    m_specularFBO->SetDrawBuffers(std::array<FramebufferObject::Attachment, 1>({ FramebufferObject::Attachment::Color0 }));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete." << std::endl;
    }

    m_specularFBO->Unbind();

    CreateSpecularFramebufferMaterial();
}
void ViewerApplication::InitializeColorFBO()
{
    m_colorFBO = std::make_shared<FramebufferObject>();
    m_colorFBO->Bind();

    m_colorTexture = std::make_shared<Texture2DObject>();
    m_colorTexture->Bind();
    m_colorTexture->SetImage(0, 1024, 1024, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    m_colorTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_colorTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);
    m_colorTexture->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
    m_colorTexture->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
    m_colorTexture->Unbind();

    m_colorFBO->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Color0, *m_colorTexture);
    m_colorFBO->SetDrawBuffers(std::array<FramebufferObject::Attachment, 1>({ FramebufferObject::Attachment::Color0 }));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete." << std::endl;
    }

    m_colorFBO->Unbind();

    CreateColorFramebufferMaterial();
}
void ViewerApplication::InitializeReflectionColorFBO()
{
    m_reflectionColorFBO = std::make_shared<FramebufferObject>();
    m_reflectionColorFBO->Bind();

    m_reflectionColorTexture = std::make_shared<Texture2DObject>();
    m_reflectionColorTexture->Bind();
    m_reflectionColorTexture->SetImage(0, 1024, 1024, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    m_reflectionColorTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_reflectionColorTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);
    m_reflectionColorTexture->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
    m_reflectionColorTexture->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
    m_reflectionColorTexture->Unbind();

    m_reflectionColorFBO->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Color0, *m_reflectionColorTexture);
    m_reflectionColorFBO->SetDrawBuffers(std::array<FramebufferObject::Attachment, 1>({ FramebufferObject::Attachment::Color0 }));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete." << std::endl;
    }

    m_reflectionColorFBO->Unbind();

    CreateReflectionColorFramebufferMaterial();
}
void ViewerApplication::InitializeBlurFBO()
{
    m_blurFBO = std::make_shared<FramebufferObject>();
    m_blurFBO->Bind();

    m_blurTexture = std::make_shared<Texture2DObject>();
    m_blurTexture->Bind();
    m_blurTexture->SetImage(0, 1024, 1024, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    m_blurTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_blurTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);
    m_blurTexture->SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_EDGE);
    m_blurTexture->SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_EDGE);
    m_blurTexture->Unbind();

    m_blurFBO->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Color0, *m_blurTexture);
    m_blurFBO->SetDrawBuffers(std::array<FramebufferObject::Attachment, 1>({ FramebufferObject::Attachment::Color0 }));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete." << std::endl;
    }

    m_blurFBO->Unbind();

    CreateBlurFramebufferMaterial();
}
void ViewerApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    // Add debug controls for light properties
    ImGui::ColorEdit3("Ambient color", &m_ambientColor[0]);
    ImGui::Separator();
    ImGui::DragFloat3("Light position", &m_lightPosition[0], 0.1f);
    ImGui::ColorEdit3("Light color", &m_lightColor[0]);
    ImGui::DragFloat("Light intensity", &m_lightIntensity, 0.05f, 0.0f, 100.0f);
    ImGui::Separator();
    ImGui::DragFloat("Specular exponent (grass)", &m_specularExponentGrass, 1.0f, 0.0f, 1000.0f);

    m_imGui.EndFrame();
}

void ViewerApplication::UpdateCamera()
{
    Window& window = GetMainWindow();

    // Update if camera is enabled (controlled by SPACE key)
    {
        bool enablePressed = window.IsKeyPressed(GLFW_KEY_SPACE);
        if (enablePressed && !m_cameraEnablePressed)
        {
            m_cameraEnabled = !m_cameraEnabled;

            window.SetMouseVisible(!m_cameraEnabled);
            m_mousePosition = window.GetMousePosition(true);
        }
        m_cameraEnablePressed = enablePressed;
    }

    if (!m_cameraEnabled)
        return;

    glm::mat4 viewTransposedMatrix = glm::transpose(m_camera.GetViewMatrix());
    glm::vec3 viewRight = viewTransposedMatrix[0];
    glm::vec3 viewForward = -viewTransposedMatrix[2];

    // Update camera translation
    {
        glm::vec2 inputTranslation(0.0f);

        if (window.IsKeyPressed(GLFW_KEY_A))
            inputTranslation.x = -1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_D))
            inputTranslation.x = 1.0f;

        if (window.IsKeyPressed(GLFW_KEY_W))
            inputTranslation.y = 1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_S))
            inputTranslation.y = -1.0f;

        inputTranslation *= m_cameraTranslationSpeed;
        inputTranslation *= GetDeltaTime();

        // Double speed if SHIFT is pressed
        if (window.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
            inputTranslation *= 2.0f;

        m_cameraPosition += inputTranslation.x * viewRight + inputTranslation.y * viewForward;
    }

    // Update camera rotation
   {
        glm::vec2 mousePosition = window.GetMousePosition(true);
        glm::vec2 deltaMousePosition = mousePosition - m_mousePosition;
        m_mousePosition = mousePosition;

        glm::vec3 inputRotation(-deltaMousePosition.x, deltaMousePosition.y, 0.0f);

        inputRotation *= m_cameraRotationSpeed;

        viewForward = glm::rotate(inputRotation.x, glm::vec3(0,1,0)) * glm::rotate(inputRotation.y, glm::vec3(viewRight)) * glm::vec4(viewForward, 0);
    }

   // Update view matrix
   m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);
}
