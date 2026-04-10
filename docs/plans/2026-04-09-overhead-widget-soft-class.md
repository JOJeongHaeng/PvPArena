# Overhead Widget Soft Class Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Stop eagerly loading the overhead status widget blueprint class in the character constructor.

**Architecture:** Convert the overhead widget class path to a soft class property on `APvPArenaCharacter`, then resolve and assign it right before `UWidgetComponent::InitWidget()` is used.

**Tech Stack:** Unreal Engine 5 C++, `TSoftClassPtr`, widget component initialization, automation tests.
