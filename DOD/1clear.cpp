#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

SDL_GPUDevice* device;

SDL_Window* window;

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	// close the window on request
	if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
	{
		return SDL_APP_SUCCESS;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	// create a window
	window = SDL_CreateWindow("Culoarea violet!", 960, 540, SDL_WINDOW_RESIZABLE);

	// create the device
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, NULL);
	SDL_ClaimWindowForGPUDevice(device, window);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	// destroy the GPU device
	SDL_DestroyGPUDevice(device);

	// destroy the window
	SDL_DestroyWindow(window);
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
	// acquire the command buffer
	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);

	// get the swapchain texture
	SDL_GPUTexture* swapchainTexture;
	Uint32 width, height;
	SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, &width, &height);

	// end the frame early if a swapchain texture is not available
	if (swapchainTexture == NULL)
	{
		// you must always submit the command buffer
		SDL_SubmitGPUCommandBuffer(commandBuffer);
		return SDL_APP_CONTINUE;
	}

	// create the color target
	SDL_GPUColorTargetInfo colorTargetInfo{};
	int red = 255, green = 0, blue = 200, transparency = 255;
	colorTargetInfo.clear_color = { red / 255.0f, green / 255.0f, blue / 255.0f, transparency / 255.0f };
	colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	colorTargetInfo.texture = swapchainTexture;

	// begin a render pass
	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);

	// draw something

	// end the render pass
	SDL_EndGPURenderPass(renderPass);

	// submit the command buffer
	SDL_SubmitGPUCommandBuffer(commandBuffer);

	return SDL_APP_CONTINUE;
}
