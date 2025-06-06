/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2023 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "LayoutPools.h"
#include "../../../Include/RmlUi/Core/Element.h"
#include "../ControlledLifetimeResource.h"
#include "../Pool.h"
#include "BlockContainer.h"
#include "FloatedBoxSpace.h"
#include "FormattingContext.h"
#include "InlineBox.h"
#include "InlineContainer.h"
#include "LineBox.h"
#include "ReplacedFormattingContext.h"
#include <algorithm>
#include <cstddef>

namespace Rml {

template <size_t Size>
struct LayoutChunk {
	alignas(std::max_align_t) byte buffer[Size];
};

static constexpr size_t ChunkSizeBig = std::max({sizeof(BlockContainer)});
static constexpr size_t ChunkSizeMedium =
	std::max({sizeof(InlineContainer), sizeof(InlineBox), sizeof(RootBox), sizeof(FlexContainer), sizeof(TableWrapper)});
static constexpr size_t ChunkSizeSmall =
	std::max({sizeof(ReplacedBox), sizeof(InlineLevelBox_Text), sizeof(InlineLevelBox_Atomic), sizeof(LineBox), sizeof(FloatedBoxSpace)});

struct LayoutPoolsData {
	Pool<LayoutChunk<ChunkSizeBig>> layout_chunk_pool_big{50, true};
	Pool<LayoutChunk<ChunkSizeMedium>> layout_chunk_pool_medium{50, true};
	Pool<LayoutChunk<ChunkSizeSmall>> layout_chunk_pool_small{50, true};
};

static ControlledLifetimeResource<LayoutPoolsData> layout_pools_data;

void LayoutPools::Initialize()
{
	layout_pools_data.Initialize();
}
void LayoutPools::Shutdown()
{
	layout_pools_data.Shutdown();
}

void* LayoutPools::AllocateLayoutChunk(size_t size)
{
	static_assert(ChunkSizeBig > ChunkSizeMedium && ChunkSizeMedium > ChunkSizeSmall, "The following assumes a strict ordering of the chunk sizes.");

	// Note: If any change is made here, make sure a corresponding change is applied to the deallocation procedure below.
	if (size <= ChunkSizeSmall)
		return layout_pools_data->layout_chunk_pool_small.AllocateAndConstruct();
	else if (size <= ChunkSizeMedium)
		return layout_pools_data->layout_chunk_pool_medium.AllocateAndConstruct();
	else if (size <= ChunkSizeBig)
		return layout_pools_data->layout_chunk_pool_big.AllocateAndConstruct();

	RMLUI_ERROR;
	return nullptr;
}

void LayoutPools::DeallocateLayoutChunk(void* chunk, size_t size)
{
	// Note: If any change is made here, make sure a corresponding change is applied to the allocation procedure above.
	if (size <= ChunkSizeSmall)
		layout_pools_data->layout_chunk_pool_small.DestroyAndDeallocate((LayoutChunk<ChunkSizeSmall>*)chunk);
	else if (size <= ChunkSizeMedium)
		layout_pools_data->layout_chunk_pool_medium.DestroyAndDeallocate((LayoutChunk<ChunkSizeMedium>*)chunk);
	else if (size <= ChunkSizeBig)
		layout_pools_data->layout_chunk_pool_big.DestroyAndDeallocate((LayoutChunk<ChunkSizeBig>*)chunk);
	else
	{
		RMLUI_ERROR;
	}
}

} // namespace Rml
