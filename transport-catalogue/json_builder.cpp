#include "json_builder.h"

namespace json {

	using namespace std::literals;

	// NodeVisitor

    Node NodeVisitor::operator()(std::nullptr_t value){
		return value;
	}

    Node NodeVisitor::operator()(const Array& value){
		return value;
	}
	
    Node NodeVisitor::operator()(const Dict& value){
		return value;
	}
	
    Node NodeVisitor::operator()(bool value){
		return value;
	}
	
    Node NodeVisitor::operator()(int value){
		return value;
	}
	
    Node NodeVisitor::operator()(double value){
		return value;
	}
	
    Node NodeVisitor::operator()(const std::string& value){
		return value;
	}
	

    // Builder

	KeyBaseContext Builder::Key(std::string key) {
		if (!is_empty_ && !nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			Dict& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
			nodes_stack_.emplace_back(&dict[std::move(key)]);
			return *this;
		}

		throw std::logic_error("Incorrect place for key : "s + key);
	}

	Builder& Builder::Value(Node::Value value, bool is_beginning) {
		if (is_empty_) {
			is_empty_ = false;
			node_ = std::move(std::visit(NodeVisitor{}, value));
			if (is_beginning) {
				nodes_stack_.push_back(&node_);
			}
			return *this;
		}

		if (!nodes_stack_.empty()) {
			if (nodes_stack_.back()->IsNull()) {
				*nodes_stack_.back() = std::move(std::visit(NodeVisitor{}, value));
				if (!is_beginning) {
					nodes_stack_.pop_back();
				}
				return *this;
			}

			if (nodes_stack_.back()->IsArray()) {
				Array& arr = const_cast<Array&>(nodes_stack_.back()->AsArray());
				arr.push_back(std::visit(NodeVisitor{}, value));
				if (is_beginning) {
					nodes_stack_.push_back(&arr.back());
				}
				return *this;
			}
		}
		throw std::logic_error("Incorrect place for value"s);
	}

	DictBaseContext Builder::StartDict() {
		Value(Dict{}, true);
		return *this;
	}

	ArrayBaseContext Builder::StartArray() {
		Value(Array{}, true);
		return *this;
	}

	Builder& Builder::EndDict() {
		if (!is_empty_ && !nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			nodes_stack_.pop_back();
			return *this;
		}
		throw std::logic_error("Incorrect place for EndDict"s);
	}

	Builder& Builder::EndArray() {
		if (!is_empty_ && !nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			nodes_stack_.pop_back();
			return *this;
		}
		throw std::logic_error("Incorrect place for EndArray"s);
	}

	Node Builder::Build() {
		if (!is_empty_ &&  nodes_stack_.empty()) {
			return std::move(node_);
		}
		throw std::logic_error("Builder is invalid"s);
	}
    // BaseContext 

	KeyBaseContext BaseContext::Key(std::string key) {
		return builder_.Key(std::move(key));
	}

	DictBaseContext BaseContext::StartDict() {
		return builder_.StartDict();
	}
	ArrayBaseContext BaseContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& BaseContext::EndDict() {
		return builder_.EndDict();
	}

	Builder& BaseContext::EndArray() {
		return builder_.EndArray();
	}

    // KeyBaseContext

	DictBaseContext KeyBaseContext::Value(Node::Value value) {
		return builder_.Value(value);
	}

    // ArrayBaseContext

    ArrayBaseContext ArrayBaseContext::Value(Node::Value value) {
		return builder_.Value(value);
	}
}