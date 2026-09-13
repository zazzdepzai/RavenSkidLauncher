#!/data/data/com.termux/files/usr/bin/bash
# ==========================================================
#   RavenSkidLauncher - Termux Menu Tool (FULL FIX)
#   Compatible: bash / sh / mksh (Termux)
# ==========================================================

REPO="https://github.com/zazzdepzai/RavenSkidLauncher.git"
GIT_NAME="zazzdepzai"
GIT_EMAIL="pphucdeptrai4@gmail.com"

# ============ MÀU SẮC ============
R="\033[1;31m"
G="\033[1;32m"
Y="\033[1;33m"
B="\033[1;34m"
C="\033[1;36m"
W="\033[1;37m"
N="\033[0m"

# ============ HÀM TIỆN ÍCH ============
pause() {
    printf "\n"
    printf "Nhấn Enter để tiếp tục..."
    read -r _dummy
}

line() {
    printf "${B}==========================================${N}\n"
}

msg() {
    # msg "màu" "nội dung"
    printf "${1}%s${N}\n" "$2"
}

# ============ MENU ============
show_menu() {
    clear
    line
    printf "${C}   RavenSkidLauncher - Termux Tool${N}\n"
    line
    printf "\n"
    printf "  ${Y}[1]${N} ${R}DELETE${N}   - Xóa repo (reset hoàn toàn)\n"
    printf "  ${Y}[2]${N} ${G}REUPLOAD${N} - Đẩy code lên GitHub (force)\n"
    printf "  ${Y}[3]${N} ${B}SETUP${N}    - Cấu hình Git + Token\n"
    printf "  ${Y}[4]${N} ${W}NEXT${N}     - Thoát chương trình\n"
    printf "\n"
    line
}

# ============ CHỨC NĂNG 1: DELETE ============
do_delete() {
    clear
    line
    printf "${R}   CHẾ ĐỘ DELETE${N}\n"
    line
    printf "\n"
    printf "${Y}CẢNH BÁO:${N} Sẽ XÓA TOÀN BỘ:\n"
    printf "  - Thư mục .git (lịch sử commit)\n"
    printf "  - File cấu hình Git local\n"
    printf "  - Có thể xóa luôn source code nếu chọn\n"
    printf "\n"
    printf "Type DELETE to confirm: "
    read -r CONFIRM

    if [ "$CONFIRM" != "DELETE" ]; then
        msg "$G" "Đã hủy."
        pause
        return
    fi

    printf "\n"
    printf "Xóa luôn source code? (y/N): "
    read -r DEL_SRC

    printf "\n"
    msg "$Y" "[*] Đang xóa .git ..."
    if rm -rf .git 2>/dev/null; then
        msg "$G" "    ✓ Đã xóa .git"
    else
        msg "$R" "    ✗ Lỗi xóa .git"
    fi

    rm -f .gitignore.bak .gitconfig 2>/dev/null

    case "$DEL_SRC" in
        y|Y|yes|YES|Yes)
            msg "$Y" "[*] Đang xóa toàn bộ source code..."
            find . -mindepth 1 -maxdepth 1 ! -name "$(basename "$0")" -exec rm -rf {} + 2>/dev/null
            msg "$G" "    ✓ Đã xóa source (giữ lại script)"
            ;;
        *)
            msg "$Y" "    → Bỏ qua xóa source."
            ;;
    esac

    printf "\n"
    msg "$G" "✓ DELETE hoàn tất!"
    pause
}

# ============ CHỨC NĂNG 2: REUPLOAD ============
do_reupload() {
    clear
    line
    printf "${G}   CHẾ ĐỘ REUPLOAD${N}\n"
    line
    printf "\n"
    printf "Sẽ THAY THẾ branch main trên GitHub\n"
    printf "bằng toàn bộ file trong thư mục hiện tại.\n"
    msg "$R" "File cũ không có ở local sẽ biến mất!"
    printf "\n"
    printf "Type DELETE to continue: "
    read -r CONFIRM

    if [ "$CONFIRM" != "DELETE" ]; then
        msg "$G" "Đã hủy."
        pause
        return
    fi

    printf "\n"

    # Kiểm tra git
    if ! command -v git >/dev/null 2>&1; then
        msg "$Y" "Git chưa cài. Đang cài..."
        pkg install git -y
    fi

    # [1/7] Init
    if [ ! -d ".git" ]; then
        msg "$Y" "[1/7] Initializing Git..."
        if ! git init; then
            msg "$R" "Lỗi init"
            pause
            return
        fi
    else
        msg "$G" "[1/7] Existing Git repository detected."
    fi

    # [2/7] Branch main
    msg "$Y" "[2/7] Setting branch to main..."
    if ! git branch -M main; then
        msg "$R" "Lỗi branch"
        pause
        return
    fi

    # [3/7] Remote
    msg "$Y" "[3/7] Setting remote..."
    git remote remove origin >/dev/null 2>&1
    if ! git remote add origin "$REPO"; then
        msg "$R" "Lỗi remote"
        pause
        return
    fi

    # [4/7] Clear index
    msg "$Y" "[4/7] Removing old local Git index..."
    git rm -r --cached . >/dev/null 2>&1

    # [5/7] Add all
    msg "$Y" "[5/7] Adding ALL current project files..."
    if ! git add -A; then
        msg "$R" "Lỗi add"
        pause
        return
    fi

    # [6/7] Commit
    msg "$Y" "[6/7] Creating clean replacement commit..."
    if ! git commit --allow-empty -m "Clean reupload RavenSkidLauncher"; then
        msg "$R" "Commit failed."
        pause
        return
    fi

    # [7/7] Force push
    msg "$Y" "[7/7] FORCE PUSHING to GitHub..."
    printf "\n"
    if git push -u origin main --force; then
        printf "\n"
        line
        msg "$G" "   ✓ SUCCESS"
        line
        printf "Repo: ${C}%s${N}\n" "$REPO"
    else
        printf "\n"
        line
        msg "$R" "   ✗ PUSH FAILED"
        line
        msg "$Y" "Gợi ý: Chạy [3] SETUP để cấu hình token"
    fi
    pause
}

# ============ CHỨC NĂNG 3: SETUP GIT ============
do_setup() {
    clear
    line
    printf "${B}   CHẾ ĐỘ SETUP GIT${N}\n"
    line
    printf "\n"

    # Cài git nếu chưa có
    if ! command -v git >/dev/null 2>&1; then
        msg "$Y" "Đang cài Git..."
        pkg install git -y
    fi

    # Cấu hình tên + email
    printf "${C}[1] Cấu hình user.name và user.email${N}\n"
    printf "  Nhập tên GitHub [%s]: " "$GIT_NAME"
    read -r INPUT_NAME
    [ -z "$INPUT_NAME" ] && INPUT_NAME="$GIT_NAME"

    printf "  Nhập email GitHub [%s]: " "$GIT_EMAIL"
    read -r INPUT_EMAIL
    [ -z "$INPUT_EMAIL" ] && INPUT_EMAIL="$GIT_EMAIL"

    git config --global user.name "$INPUT_NAME"
    git config --global user.email "$INPUT_EMAIL"
    msg "$G" "    ✓ Đã set: $INPUT_NAME <$INPUT_EMAIL>"
    printf "\n"

    # Token
    printf "${C}[2] Cấu hình Personal Access Token${N}\n"
    msg "$Y" "  Tạo token tại: https://github.com/settings/tokens"
    msg "$Y" "  Tick quyền: repo"
    printf "\n"
    printf "  Dán token (ẩn): "
    stty -echo 2>/dev/null
    read -r TOKEN
    stty echo 2>/dev/null
    printf "\n"

    if [ -z "$TOKEN" ]; then
        msg "$Y" "  Bỏ qua token."
    else
        git config --global credential.helper store
        printf "https://%s:%s@github.com\n" "$INPUT_NAME" "$TOKEN" > "$HOME/.git-credentials"
        chmod 600 "$HOME/.git-credentials"
        msg "$G" "    ✓ Đã lưu token"

        if [ -d ".git" ]; then
            git remote set-url origin "https://${TOKEN}@github.com/zazzdepzai/RavenSkidLauncher.git" 2>/dev/null
            msg "$G" "    ✓ Đã gắn token vào remote hiện tại"
        fi
    fi
    printf "\n"

    # Cấu hình bổ sung
    printf "${C}[3] Cấu hình bổ sung${N}\n"
    git config --global --add safe.directory '*'
    git config --global http.postBuffer 524288000
    git config --global init.defaultBranch main
    msg "$G" "    ✓ safe.directory, postBuffer, defaultBranch"
    printf "\n"

    # Test kết nối
    printf "${C}[4] Kiểm tra kết nối GitHub...${N}\n"
    if git ls-remote "$REPO" >/dev/null 2>&1; then
        msg "$G" "    ✓ Kết nối GitHub OK"
    else
        msg "$R" "    ✗ Không kết nối được (kiểm tra mạng/token)"
    fi

    pause
}

# ============ MAIN LOOP ============
while true; do
    show_menu
    printf "Chọn [1-4]: "
    read -r CHOICE

    case "$CHOICE" in
        1) do_delete ;;
        2) do_reupload ;;
        3) do_setup ;;
        4)
            clear
            msg "$G" "Tạm biệt! 👋"
            exit 0
            ;;
        *)
            msg "$R" "Lựa chọn không hợp lệ!"
            sleep 1
            ;;
    esac
done