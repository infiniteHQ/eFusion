#pragma once

#include "../../../../lib/vortex/lib/cherry/cherry.hpp"
#include "../../../../lib/vortex/lib/cherry/platform/engine/app/app.hpp"
#include "../../../../lib/vortex/lib/cherry/platform/engine/components/components.hpp"
#include "../../../../lib/vortex/main/include/vortex.h"
#include "../../../../lib/vortex/main/include/vortex_internals.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <regex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifndef TEXT_EDITOR_CORE_HPP
#define TEXT_EDITOR_CORE_HPP

using namespace Cherry;

namespace ModuleUI {

class TextEditorCore {
public:
  enum class PaletteId { Dark, Light, Mariana, RetroBlue };
  enum class LanguageDefinitionId {
    None,
    Cpp,
    C,
    Cs,
    Python,
    Lua,
    Json,
    Sql,
    AngelScript,
    Glsl,
    Hlsl
  };

  enum class PaletteIndex {
    Default,
    Keyword,
    Number,
    String,
    CharLiteral,
    Punctuation,
    Preprocessor,
    Identifier,
    KnownIdentifier,
    PreprocIdentifier,
    Comment,
    MultiLineComment,
    Background,
    Cursor,
    Selection,
    ErrorMarker,
    Breakpoint,
    LineNumber,
    CurrentLineFill,
    CurrentLineFillInactive,
    CurrentLineEdge,
    Max
  };

  enum class SelectionMode { Normal, Word, Line };

  struct Breakpoint {
    int mLine;
    bool mEnabled;
    std::string mCondition;

    Breakpoint() : mLine(-1), mEnabled(false) {}
  };

  // Represents a character coordinate from the user's point of view,
  // i. e. consider an uniform grid (assuming fixed-width font) on the
  // screen as it is rendered, and each cell has its own coordinate, starting
  // from 0. Tabs are counted as [1..mTabSize] count empty spaces, depending on
  // how many space is necessary to reach the next tab stop.
  // For example, coordinate (1, 5) represents the character 'B' in a line
  // "\tABC", when mTabSize = 4, because it is rendered as "    ABC" on the
  // screen.
  struct Coordinates {
    int mLine, mColumn;
    Coordinates() : mLine(0), mColumn(0) {}
    Coordinates(int aLine, int aColumn) : mLine(aLine), mColumn(aColumn) {
      assert(aLine >= 0);
      assert(aColumn >= 0);
    }
    static Coordinates Invalid() {
      static Coordinates invalid(-1, -1);
      return invalid;
    }

    bool operator==(const Coordinates &o) const {
      return mLine == o.mLine && mColumn == o.mColumn;
    }

    bool operator!=(const Coordinates &o) const {
      return mLine != o.mLine || mColumn != o.mColumn;
    }

    bool operator<(const Coordinates &o) const {
      if (mLine != o.mLine)
        return mLine < o.mLine;
      return mColumn < o.mColumn;
    }

    bool operator>(const Coordinates &o) const {
      if (mLine != o.mLine)
        return mLine > o.mLine;
      return mColumn > o.mColumn;
    }

    bool operator<=(const Coordinates &o) const {
      if (mLine != o.mLine)
        return mLine < o.mLine;
      return mColumn <= o.mColumn;
    }

    bool operator>=(const Coordinates &o) const {
      if (mLine != o.mLine)
        return mLine > o.mLine;
      return mColumn >= o.mColumn;
    }
  };

  struct Identifier {
    Coordinates mLocation;
    std::string mDeclaration;
  };

  typedef std::string String;
  typedef std::unordered_map<std::string, Identifier> Identifiers;
  typedef std::unordered_set<std::string> Keywords;
  typedef std::map<int, std::string> ErrorMarkers;
  typedef std::unordered_set<int> Breakpoints;
  typedef std::array<ImU32, (unsigned)PaletteIndex::Max> Palette;
  typedef uint8_t Char;

  struct Glyph {
    Char mChar;
    PaletteIndex mColorIndex = PaletteIndex::Default;
    bool mComment : 1;
    bool mMultiLineComment : 1;
    bool mPreprocessor : 1;

    Glyph(Char aChar, PaletteIndex aColorIndex)
        : mChar(aChar), mColorIndex(aColorIndex), mComment(false),
          mMultiLineComment(false), mPreprocessor(false) {}
  };

  typedef std::vector<Glyph> Line;
  typedef std::vector<Line> Lines;

  struct LanguageDefinition {
    typedef std::pair<std::string, PaletteIndex> TokenRegexString;
    typedef std::vector<TokenRegexString> TokenRegexStrings;
    typedef bool (*TokenizeCallback)(const char *in_begin, const char *in_end,
                                     const char *&out_begin,
                                     const char *&out_end,
                                     PaletteIndex &paletteIndex);

    std::string mName;
    Keywords mKeywords;
    Identifiers mIdentifiers;
    Identifiers mPreprocIdentifiers;
    std::string mCommentStart, mCommentEnd, mSingleLineComment;
    char mPreprocChar;
    bool mAutoIndentation;

    TokenizeCallback mTokenize;

    TokenRegexStrings mTokenRegexStrings;

    bool mCaseSensitive;

    LanguageDefinition()
        : mPreprocChar('#'), mAutoIndentation(true), mTokenize(nullptr),
          mCaseSensitive(true) {}

    static const LanguageDefinition &CPlusPlus();
    static const LanguageDefinition &HLSL();
    static const LanguageDefinition &GLSL();
    static const LanguageDefinition &C();
    static const LanguageDefinition &SQL();
    static const LanguageDefinition &AngelScript();
    static const LanguageDefinition &Lua();
  };

  // TextEditorCore(const std::filesystem::path& PathToFile);
  TextEditorCore(const std::string &buffer);
  ~TextEditorCore();

  void render();
  void close();

  bool opened;
  std::string name;
  // std::shared_ptr<Walnut::Image> m_MainIcon;

  void SetLanguageDefinition(const LanguageDefinition &aLanguageDef);
  const LanguageDefinition &GetLanguageDefinition() const {
    return mLanguageDefinition;
  }

  const Palette &GetPalette() const { return mPaletteBase; }
  void SetPalette(const Palette &aValue);

  void SetErrorMarkers(const ErrorMarkers &aMarkers) {
    mErrorMarkers = aMarkers;
  }
  void SetBreakpoints(const Breakpoints &aMarkers) { mBreakpoints = aMarkers; }

  void Render(const char *aTitle, const ImVec2 &aSize = ImVec2(),
              bool aBorder = false);
  void SetText(const std::string &aText);
  std::string GetText() const;

  void SetTextLines(const std::vector<std::string> &aLines);
  std::vector<std::string> GetTextLines() const;

  std::string GetSelectedText() const;
  std::string GetCurrentLineText() const;

  int GetTotalLines() const { return (int)mLines.size(); }
  bool IsOverwrite() const { return mOverwrite; }

  void SetReadOnly(bool aValue);
  bool IsReadOnly() const { return mReadOnly; }
  bool IsTextChanged() const { return mTextChanged; }
  bool IsCursorPositionChanged() const { return mCursorPositionChanged; }

  bool IsColorizerEnabled() const { return mColorizerEnabled; }
  void SetColorizerEnable(bool aValue);

  Coordinates GetCursorPosition() const { return GetActualCursorCoordinates(); }
  void SetCursorPosition(const Coordinates &aPosition);

  inline void SetHandleMouseInputs(bool aValue) { mHandleMouseInputs = aValue; }
  inline bool IsHandleMouseInputsEnabled() const {
    return mHandleKeyboardInputs;
  }

  inline void SetHandleKeyboardInputs(bool aValue) {
    mHandleKeyboardInputs = aValue;
  }
  inline bool IsHandleKeyboardInputsEnabled() const {
    return mHandleKeyboardInputs;
  }

  inline void SetImGuiChildIgnored(bool aValue) { mIgnoreImGuiChild = aValue; }
  inline bool IsImGuiChildIgnored() const { return mIgnoreImGuiChild; }

  inline void SetShowWhitespaces(bool aValue) { mShowWhitespaces = aValue; }
  inline bool IsShowingWhitespaces() const { return mShowWhitespaces; }

  inline void SetReadOnlyEnabled(bool aValue) { mReadOnly = aValue; }
  inline bool IsReadOnlyEnabled() const { return mReadOnly; }
  inline void SetAutoIndentEnabled(bool aValue) { mAutoIndent = aValue; }
  inline bool IsAutoIndentEnabled() const { return mAutoIndent; }
  inline void SetShowWhitespacesEnabled(bool aValue) {
    mShowWhitespaces = aValue;
  }
  inline bool IsShowWhitespacesEnabled() const { return mShowWhitespaces; }
  inline void SetShowLineNumbersEnabled(bool aValue) {
    mShowLineNumbers = aValue;
  }
  inline bool IsShowLineNumbersEnabled() const { return mShowLineNumbers; }
  inline void SetShortTabsEnabled(bool aValue) { mShortTabs = aValue; }
  inline bool IsShortTabsEnabled() const { return mShortTabs; }
  inline int GetLineCount() const { return mLines.size(); }
  inline bool IsOverwriteEnabled() const { return mOverwrite; }
  void SetTabSize(int aValue);
  inline int GetTabSize() const { return mTabSize; }

  void InsertText(const std::string &aValue);
  void InsertText(const char *aValue);

  void SwitchToFile(std::string PathToFile);

  void MoveUp(int aAmount = 1, bool aSelect = false);
  void MoveDown(int aAmount = 1, bool aSelect = false);
  void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
  void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
  void MoveTop(bool aSelect = false);
  void MoveBottom(bool aSelect = false);
  void MoveHome(bool aSelect = false);
  void MoveEnd(bool aSelect = false);

  void SetSelectionStart(const Coordinates &aPosition);
  void SetSelectionEnd(const Coordinates &aPosition);
  void SetSelection(const Coordinates &aStart, const Coordinates &aEnd,
                    SelectionMode aMode = SelectionMode::Normal);
  void SelectWordUnderCursor();
  void SelectAll();
  bool HasSelection() const;

  void Copy();
  void Cut();
  void Paste();
  void Delete();

  bool CanUndo() const;
  bool CanRedo() const;
  void Undo(int aSteps = 1);
  void Redo(int aSteps = 1);

  static const Palette &GetDarkPalette();
  static const Palette &GetLightPalette();
  static const Palette &GetRetroBluePalette();
  // float mLineSpacing;
  int mTabSize;

  float mLineSpacing = 1.0f;
  bool mAutoIndent = true;
  bool mShowLineNumbers = true;
  bool mShortTabs = false;

private:
  typedef std::vector<std::pair<std::regex, PaletteIndex>> RegexList;

  struct EditorState {
    Coordinates mSelectionStart;
    Coordinates mSelectionEnd;
    Coordinates mCursorPosition;
  };

  class UndoRecord {
  public:
    UndoRecord() {}
    ~UndoRecord() {}

    UndoRecord(const std::string &aAdded,
               const TextEditorCore::Coordinates aAddedStart,
               const TextEditorCore::Coordinates aAddedEnd,

               const std::string &aRemoved,
               const TextEditorCore::Coordinates aRemovedStart,
               const TextEditorCore::Coordinates aRemovedEnd,

               TextEditorCore::EditorState &aBefore,
               TextEditorCore::EditorState &aAfter);

    void Undo(TextEditorCore *aEditor);
    void Redo(TextEditorCore *aEditor);

    std::string mAdded;
    Coordinates mAddedStart;
    Coordinates mAddedEnd;

    std::string mRemoved;
    Coordinates mRemovedStart;
    Coordinates mRemovedEnd;

    EditorState mBefore;
    EditorState mAfter;
  };

  typedef std::vector<UndoRecord> UndoBuffer;

  void ProcessInputs();
  void Colorize(int aFromLine = 0, int aCount = -1);
  void ColorizeRange(int aFromLine = 0, int aToLine = 0);
  void ColorizeInternal();
  float TextDistanceToLineStart(const Coordinates &aFrom) const;
  void EnsureCursorVisible();
  int GetPageSize() const;
  std::string GetText(const Coordinates &aStart, const Coordinates &aEnd) const;
  Coordinates GetActualCursorCoordinates() const;
  Coordinates SanitizeCoordinates(const Coordinates &aValue) const;
  void Advance(Coordinates &aCoordinates) const;
  void DeleteRange(const Coordinates &aStart, const Coordinates &aEnd);
  int InsertTextAt(Coordinates &aWhere, const char *aValue);
  void AddUndo(UndoRecord &aValue);
  Coordinates ScreenPosToCoordinates(const ImVec2 &aPosition) const;
  Coordinates FindWordStart(const Coordinates &aFrom) const;
  Coordinates FindWordEnd(const Coordinates &aFrom) const;
  Coordinates FindNextWord(const Coordinates &aFrom) const;
  int GetCharacterIndex(const Coordinates &aCoordinates) const;
  int GetCharacterColumn(int aLine, int aIndex) const;
  int GetLineCharacterCount(int aLine) const;
  int GetLineMaxColumn(int aLine) const;
  bool IsOnWordBoundary(const Coordinates &aAt) const;
  void RemoveLine(int aStart, int aEnd);
  void RemoveLine(int aIndex);
  Line &InsertLine(int aIndex);
  void EnterCharacter(ImWchar aChar, bool aShift);
  void Backspace();
  void DeleteSelection();
  std::string GetWordUnderCursor() const;
  std::string GetWordAt(const Coordinates &aCoords) const;
  ImU32 GetGlyphColor(const Glyph &aGlyph) const;

  void HandleKeyboardInputs();
  void HandleMouseInputs();
  void Render();

  Lines mLines;
  EditorState mState;
  UndoBuffer mUndoBuffer;
  int mUndoIndex;

  bool mOverwrite;
  bool mReadOnly;
  bool mWithinRender;
  bool mScrollToCursor;
  bool mScrollToTop;
  bool mTextChanged;
  bool mColorizerEnabled;
  float mTextStart; // position (in pixels) where a code line starts relative to
                    // the left of the TextEditor.
  int mLeftMargin;
  bool mCursorPositionChanged;
  int mColorRangeMin, mColorRangeMax;
  SelectionMode mSelectionMode;
  bool mHandleKeyboardInputs;
  bool mHandleMouseInputs;
  bool mIgnoreImGuiChild;
  bool mShowWhitespaces;

  Palette mPaletteBase;
  Palette mPalette;
  LanguageDefinition mLanguageDefinition;
  RegexList mRegexList;

  bool mCheckComments;
  Breakpoints mBreakpoints;
  ErrorMarkers mErrorMarkers;
  ImVec2 mCharAdvance;
  Coordinates mInteractiveStart, mInteractiveEnd;
  std::string mLineBuffer;
  uint64_t mStartTime;

  float mLastClick;
};

class TextAreaComponent : public Component {
public:
  TextAreaComponent(const Cherry::Identifier &id, float *width, float *height,
                    std::string *buffer)
      : Component(id), m_Width(width), m_Height(height), m_TextEditor("temp"),
        m_EditBuffer(buffer) {
    // Identifier
    SetIdentifier(id);
    // Colors
    SetProperty("color_text", "theme:text_area_color_text");
    SetProperty("undo_pending", "false");
    SetProperty("redo_pending", "false");
    SetProperty("save_pending", "false");
    SetProperty("refresh_pending", "false");

    SetData("save_ready", "false");
    SetData("text_changed", false);

    if (buffer) {
      m_TextEditor.SetText(*buffer);
    }
  }

  void Render() override {
    if (!m_Width || !m_Height) {
      return;
    }
    Cherry::PushFont("FiraCode");

    if (GetProperty("refresh_pending") == "true") {
      if (m_EditBuffer) {
        m_TextEditor.SetText(*m_EditBuffer);
      }
      SetProperty("refresh_pending", "false");
    }

    if (m_TextEditor.IsTextChanged()) {
      SetData("text_changed", true);
    } else {
      SetData("text_changed", false);
    }

    if (GetProperty("save_pending") == "true") {
      if (m_EditBuffer) {
        *m_EditBuffer = m_TextEditor.GetText();
        SetData("save_ready", "true");
      }
      SetProperty("save_pending", "false");
    }

    if (GetPropertyAs<bool>("undo_pending")) {
      if (m_TextEditor.CanUndo()) {
        m_TextEditor.Undo();
      }
      SetProperty("undo_pending", false);
    }

    if (GetPropertyAs<bool>("redo_pending")) {
      if (m_TextEditor.CanRedo()) {
        m_TextEditor.Redo();
      }
      SetProperty("redo_pending", false);
    }

    m_TextEditor.render();

    Cherry::PopFont();
  }

private:
  float *m_Width;
  float *m_Height;
  TextEditorCore m_TextEditor;
  std::string *m_EditBuffer;
};

inline Component &TextArea(const Identifier &identifier, float *width,
                           float *height, std::string *buffer) {
  return CherryApp.PushComponent<TextAreaComponent>(identifier, width, height,
                                                    buffer);
}

inline Component &TextArea(float *width, float *height, std::string *buffer) {
  return ModuleUI::TextArea(
      Application::GenerateUniqueID(width, height, buffer, "TextArea"), width,
      height, buffer);
}

}; // namespace ModuleUI

#endif // TEXT_EDITOR_CORE_HPP
