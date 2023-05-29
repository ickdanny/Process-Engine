#pragma once

#include <vector>

#include "Utility/Void.h"

namespace wasp::channel {

	class ChannelBase{
	public:
		virtual ~ChannelBase() = default;
	};

	template <typename T = utility::Void>
	class Channel : public ChannelBase{
	private:
		std::vector<T> messages{};

	public:
		Channel() = default;
		~Channel() override = default;

		bool hasMessages() const {
			return !messages.empty();
		}

		bool isEmpty() const {
			return messages.empty();
		}

		auto& getMessages() {
			return messages;
		}

		const auto& getMessages() const {
			return messages;
		}

		void addMessage(const T& message) {
			messages.push_back(message);
		}

		template <typename... Ts>
		void emplaceMessage(Ts&&... args) {
			messages.emplace_back(args...);
		}

		void clear() {
			messages.clear();
		}
	};

	template<>
	class Channel<utility::Void> : public ChannelBase {
	private:
		std::size_t messages{};

	public:
		Channel()
			: messages{ 0 } {
		}
		~Channel() override = default;

		bool hasMessages() const {
			return static_cast<bool>(messages);
		}

		bool isEmpty() const {
			return !static_cast<bool>(messages);
		}

		std::size_t getMessages() const {
			return messages;
		}

		void addMessage() {
			++messages;
		}

		void clear() {
			messages = 0;
		}
	};
}