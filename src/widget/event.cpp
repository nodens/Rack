#include <widget/event.hpp>
#include <widget/Widget.hpp>
#include <context.hpp>
#include <window.hpp>


namespace rack {
namespace widget {


void EventState::setHovered(widget::Widget* w) {
	if (w == hoveredWidget)
		return;

	if (hoveredWidget) {
		// Trigger LeaveEvent
		Widget::LeaveEvent eLeave;
		hoveredWidget->onLeave(eLeave);
		hoveredWidget = NULL;
	}

	if (w) {
		// Trigger EnterEvent
		EventContext cEnter;
		cEnter.target = w;
		Widget::EnterEvent eEnter;
		eEnter.context = &cEnter;
		w->onEnter(eEnter);
		hoveredWidget = cEnter.target;
	}
}

void EventState::setDragged(widget::Widget* w, int button) {
	if (w == draggedWidget)
		return;

	if (draggedWidget) {
		// Trigger DragEndEvent
		Widget::DragEndEvent eDragEnd;
		eDragEnd.button = dragButton;
		draggedWidget->onDragEnd(eDragEnd);
		draggedWidget = NULL;
	}

	dragButton = button;

	if (w) {
		// Trigger DragStartEvent
		EventContext cDragStart;
		cDragStart.target = w;
		Widget::DragStartEvent eDragStart;
		eDragStart.context = &cDragStart;
		eDragStart.button = dragButton;
		w->onDragStart(eDragStart);
		draggedWidget = cDragStart.target;
	}
}

void EventState::setDragHovered(widget::Widget* w) {
	if (w == dragHoveredWidget)
		return;

	if (dragHoveredWidget) {
		// Trigger DragLeaveEvent
		Widget::DragLeaveEvent eDragLeave;
		eDragLeave.button = dragButton;
		eDragLeave.origin = draggedWidget;
		dragHoveredWidget->onDragLeave(eDragLeave);
		dragHoveredWidget = NULL;
	}

	if (w) {
		// Trigger DragEnterEvent
		EventContext cDragEnter;
		cDragEnter.target = w;
		Widget::DragEnterEvent eDragEnter;
		eDragEnter.context = &cDragEnter;
		eDragEnter.button = dragButton;
		eDragEnter.origin = draggedWidget;
		w->onDragEnter(eDragEnter);
		dragHoveredWidget = cDragEnter.target;
	}
}

void EventState::setSelected(widget::Widget* w) {
	if (w == selectedWidget)
		return;

	if (selectedWidget) {
		// Trigger DeselectEvent
		Widget::DeselectEvent eDeselect;
		selectedWidget->onDeselect(eDeselect);
		selectedWidget = NULL;
	}

	if (w) {
		// Trigger SelectEvent
		EventContext cSelect;
		cSelect.target = w;
		Widget::SelectEvent eSelect;
		eSelect.context = &cSelect;
		w->onSelect(eSelect);
		selectedWidget = cSelect.target;
	}
}

void EventState::finalizeWidget(widget::Widget* w) {
	if (hoveredWidget == w)
		setHovered(NULL);
	if (draggedWidget == w)
		setDragged(NULL, 0);
	if (dragHoveredWidget == w)
		setDragHovered(NULL);
	if (selectedWidget == w)
		setSelected(NULL);
	if (lastClickedWidget == w)
		lastClickedWidget = NULL;
}

bool EventState::handleButton(math::Vec pos, int button, int action, int mods) {
	bool cursorLocked = APP->window->isCursorLocked();

	widget::Widget* clickedWidget = NULL;
	if (!cursorLocked) {
		// Trigger ButtonEvent
		EventContext cButton;
		Widget::ButtonEvent eButton;
		eButton.context = &cButton;
		eButton.pos = pos;
		eButton.button = button;
		eButton.action = action;
		eButton.mods = mods;
		rootWidget->onButton(eButton);
		clickedWidget = cButton.target;
	}

	if (action == GLFW_PRESS) {
		setDragged(clickedWidget, button);
	}

	if (action == GLFW_RELEASE) {
		setDragHovered(NULL);

		if (clickedWidget && draggedWidget) {
			// Trigger DragDropEvent
			Widget::DragDropEvent eDragDrop;
			eDragDrop.button = dragButton;
			eDragDrop.origin = draggedWidget;
			clickedWidget->onDragDrop(eDragDrop);
		}

		setDragged(NULL, 0);
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			setSelected(clickedWidget);
		}

		if (action == GLFW_PRESS) {
			const double doubleClickDuration = 0.3;
			double clickTime = glfwGetTime();
			if (clickedWidget
			    && clickTime - lastClickTime <= doubleClickDuration
			    && lastClickedWidget == clickedWidget) {
				// Trigger DoubleClickEvent
				Widget::DoubleClickEvent eDoubleClick;
				clickedWidget->onDoubleClick(eDoubleClick);
				// Reset double click
				lastClickTime = -INFINITY;
				lastClickedWidget = NULL;
			}
			else {
				lastClickTime = clickTime;
				lastClickedWidget = clickedWidget;
			}
		}
	}

	return !!clickedWidget;
}

bool EventState::handleHover(math::Vec pos, math::Vec mouseDelta) {
	bool cursorLocked = APP->window->isCursorLocked();

	// Fake a key RACK_HELD event for each held key
	if (!cursorLocked) {
		int mods = APP->window->getMods();
		for (int key : heldKeys) {
			int scancode = glfwGetKeyScancode(key);
			handleKey(pos, key, scancode, RACK_HELD, mods);
		}
	}

	if (draggedWidget) {
		bool dragHovered = false;
		if (!cursorLocked) {
			// Trigger DragHoverEvent
			EventContext cDragHover;
			Widget::DragHoverEvent eDragHover;
			eDragHover.context = &cDragHover;
			eDragHover.button = dragButton;
			eDragHover.pos = pos;
			eDragHover.mouseDelta = mouseDelta;
			eDragHover.origin = draggedWidget;
			rootWidget->onDragHover(eDragHover);

			setDragHovered(cDragHover.target);
			// If consumed, don't continue after DragMoveEvent so HoverEvent is not triggered.
			if (cDragHover.target)
				dragHovered = true;
		}

		// Trigger DragMoveEvent
		Widget::DragMoveEvent eDragMove;
		eDragMove.button = dragButton;
		eDragMove.mouseDelta = mouseDelta;
		draggedWidget->onDragMove(eDragMove);
		if (dragHovered)
			return true;
	}

	if (!cursorLocked) {
		// Trigger HoverEvent
		EventContext cHover;
		Widget::HoverEvent eHover;
		eHover.context = &cHover;
		eHover.pos = pos;
		eHover.mouseDelta = mouseDelta;
		rootWidget->onHover(eHover);

		setHovered(cHover.target);
		if (cHover.target)
			return true;
	}
	return false;
}

bool EventState::handleLeave() {
	heldKeys.clear();
	// When leaving the window, don't un-hover widgets because the mouse might be dragging.
	// setDragHovered(NULL);
	// setHovered(NULL);
	return true;
}

bool EventState::handleScroll(math::Vec pos, math::Vec scrollDelta) {
	// Trigger HoverScrollEvent
	EventContext cHoverScroll;
	Widget::HoverScrollEvent eHoverScroll;
	eHoverScroll.context = &cHoverScroll;
	eHoverScroll.pos = pos;
	eHoverScroll.scrollDelta = scrollDelta;
	rootWidget->onHoverScroll(eHoverScroll);

	return !!cHoverScroll.target;
}

bool EventState::handleDrop(math::Vec pos, const std::vector<std::string>& paths) {
	// Trigger PathDropEvent
	EventContext cPathDrop;
	Widget::PathDropEvent ePathDrop(paths);
	ePathDrop.context = &cPathDrop;
	ePathDrop.pos = pos;
	rootWidget->onPathDrop(ePathDrop);

	return !!cPathDrop.target;
}

bool EventState::handleText(math::Vec pos, int codepoint) {
	if (selectedWidget) {
		// Trigger SelectTextEvent
		EventContext cSelectText;
		Widget::SelectTextEvent eSelectText;
		eSelectText.context = &cSelectText;
		eSelectText.codepoint = codepoint;
		selectedWidget->onSelectText(eSelectText);
		if (cSelectText.target)
			return true;
	}

	// Trigger HoverText
	EventContext cHoverText;
	Widget::HoverTextEvent eHoverText;
	eHoverText.context = &cHoverText;
	eHoverText.pos = pos;
	eHoverText.codepoint = codepoint;
	rootWidget->onHoverText(eHoverText);

	return !!cHoverText.target;
}

bool EventState::handleKey(math::Vec pos, int key, int scancode, int action, int mods) {
	// Update heldKey state
	if (action == GLFW_PRESS) {
		heldKeys.insert(key);
	}
	else if (action == GLFW_RELEASE) {
		auto it = heldKeys.find(key);
		if (it != heldKeys.end())
			heldKeys.erase(it);
	}

	if (selectedWidget) {
		// Trigger SelectKeyEvent
		EventContext cSelectKey;
		Widget::SelectKeyEvent eSelectKey;
		eSelectKey.context = &cSelectKey;
		eSelectKey.key = key;
		eSelectKey.scancode = scancode;
		const char* keyName = glfwGetKeyName(key, scancode);
		if (keyName)
			eSelectKey.keyName = keyName;
		eSelectKey.action = action;
		eSelectKey.mods = mods;
		selectedWidget->onSelectKey(eSelectKey);
		if (cSelectKey.target)
			return true;
	}

	// Trigger HoverKeyEvent
	EventContext cHoverKey;
	Widget::HoverKeyEvent eHoverKey;
	eHoverKey.context = &cHoverKey;
	eHoverKey.pos = pos;
	eHoverKey.key = key;
	eHoverKey.scancode = scancode;
	const char* keyName = glfwGetKeyName(key, scancode);
	if (keyName)
		eHoverKey.keyName = keyName;
	eHoverKey.action = action;
	eHoverKey.mods = mods;
	rootWidget->onHoverKey(eHoverKey);
	return !!cHoverKey.target;
}

bool EventState::handleDirty() {
	// Trigger DirtyEvent
	EventContext cDirty;
	Widget::DirtyEvent eDirty;
	eDirty.context = &cDirty;
	rootWidget->onDirty(eDirty);
	return true;
}


} // namespace widget
} // namespace rack