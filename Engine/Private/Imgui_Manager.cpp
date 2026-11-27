#include "Imgui_Manager.h"


CImgui_Manager::CImgui_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:m_pDevice{ pDevice }
	,m_pContext{ pContext }
{
	Safe_AddRef(pDevice);
	Safe_AddRef(pContext);
}

HRESULT CImgui_Manager::Initialize(HWND _hWnd)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImNodes::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	if (GetFileAttributesW(L"C:/Windows/Fonts/malgun.ttf") != INVALID_FILE_ATTRIBUTES)
	{
		io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/malgun.ttf", 17.f, nullptr, io.Fonts->GetGlyphRangesKorean());
	}
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;


	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();


	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	return S_OK;
}

void CImgui_Manager::Update()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//ImGuizmo::BeginFrame();
}

void CImgui_Manager::Render()
{
	// Rendering
	ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    const ImVec2 originalSize = drawData->DisplaySize;
    ImVec2 renderSize = originalSize;
    ID3D11RenderTargetView* renderTarget = nullptr;
    m_pContext->OMGetRenderTargets(1, &renderTarget, nullptr);
    if (renderTarget)
    {
        ID3D11Resource* resource = nullptr;
        renderTarget->GetResource(&resource);
        ID3D11Texture2D* texture = nullptr;
        if (SUCCEEDED(resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture))))
        {
            D3D11_TEXTURE2D_DESC description{};
            texture->GetDesc(&description);
            renderSize = ImVec2(static_cast<float>(description.Width), static_cast<float>(description.Height));
            texture->Release();
        }
        resource->Release();
        renderTarget->Release();
    }

    // This legacy DX11 backend ignores DisplayFramebufferScale.
    // Adapt only UI draw coordinates to the fixed-size back buffer.
    auto scaleDrawData = [drawData](ImVec2 scale)
    {
        const ImVec2 origin = drawData->DisplayPos;
        for (int listIndex = 0; listIndex < drawData->CmdListsCount; ++listIndex)
        {
            ImDrawList* list = drawData->CmdLists[listIndex];
            for (ImDrawVert& vertex : list->VtxBuffer)
            {
                vertex.pos.x = origin.x + (vertex.pos.x - origin.x) * scale.x;
                vertex.pos.y = origin.y + (vertex.pos.y - origin.y) * scale.y;
            }
            for (ImDrawCmd& command : list->CmdBuffer)
            {
                command.ClipRect.x = origin.x + (command.ClipRect.x - origin.x) * scale.x;
                command.ClipRect.z = origin.x + (command.ClipRect.z - origin.x) * scale.x;
                command.ClipRect.y = origin.y + (command.ClipRect.y - origin.y) * scale.y;
                command.ClipRect.w = origin.y + (command.ClipRect.w - origin.y) * scale.y;
            }
        }
    };
    if (originalSize.x > 0.f && originalSize.y > 0.f)
    {
        scaleDrawData(ImVec2(renderSize.x / originalSize.x, renderSize.y / originalSize.y));
        drawData->DisplaySize = renderSize;
        ImGui_ImplDX11_RenderDrawData(drawData);
        scaleDrawData(ImVec2(originalSize.x / renderSize.x, originalSize.y / renderSize.y));
        drawData->DisplaySize = originalSize;
    }

	// UpdateBounding and Render additional Platform Windows
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

CImgui_Manager* CImgui_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND _hWnd)
{
	CImgui_Manager* pInstance = new CImgui_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize(_hWnd)))
	{
		MSG_BOX("Failed to Created : CHeight");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CImgui_Manager::Free()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	//ImNodes::DestroyContext();
	ImGui::DestroyContext();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
