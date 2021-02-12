// This file is part of the tetris-table project. Copyright (c) Johan Duparc.

#pragma once

#include "SenderInterface.h"
#include "shared/types.h"

#include <memory>
#include <vector>

class Command;
class SenderInterface;

class RemoteDisplay
{
public:
	bool Connect();
	bool IsConnected();

	void Clear(){ Fill({0,0,0}); }
	void Fill(const Color_24b& fillColor);

	void FrameBegin(); // geometry info ?
	void FrameEnd();

	void SetPixel(int32_t stripIndex, const Color_24b& color);


	void PrintSerialInput();

private:
	void FenceCommandQueue();
	void PushCommand(const Command& command);

private:
//---------------
	class Segment
	{
	public:
		Segment();
		void AddCommand(const Command& command);
		void Push(std::unique_ptr<class SenderInterface>& sender);

		bool IsWaitingAck() const { return buffer.empty() && canWrite == false; }
		bool IsEmpty()      const { return buffer.empty(); }
		bool Writable()     const { return canWrite; }
		void Close()              { canWrite = false; } // bad api warn: warn if already closed

		bool allowedToTransmit = false;

	private:
		std::vector<uint8_t> buffer;
		bool canWrite = true; // turned false when closed
		uint8_t id;
	};
	std::vector<Segment> segments;

	Segment& GetWriteSegment(); // imply open
	void CloseSegment(); // makes sure write segment is not
	void TrySendSegments();
//---------------

	std::unique_ptr<class SenderInterface> sender;
	bool canProcessCommandQueue = true;
};
