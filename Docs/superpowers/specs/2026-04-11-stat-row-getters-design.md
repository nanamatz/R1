# Stat Row Getters Design

## Goal
Add public getters for the attribute name text in `UR1StatUpgradeRow` and `UR1StatDetailRow` to facilitate matching in subsequent tasks.

## Architecture
- **UR1StatDetailRow**: Add `FText GetAttributeName() const`.
- **UR1StatUpgradeRow**: Add `FText GetAttributeName() const`.

## Implementation Details
### UR1StatDetailRow
- **Header**: Declare `FText GetAttributeName() const;`
- **Source**: Return `Text_AttributeName ? Text_AttributeName->GetText() : FText::GetEmpty();`

### UR1StatUpgradeRow
- **Header**: Declare `FText GetAttributeName() const;`
- **Source**: Return `Text_AttributeName ? Text_AttributeName->GetText() : FText::GetEmpty();`
- **Includes**: Ensure `Components/Button.h` is included in the source file.

## Testing
- Verify that the new methods can be called and return the expected text.
- Verify that the project builds correctly.
