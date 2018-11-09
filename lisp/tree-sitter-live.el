;;; tree-sitter.el --- Common definitions for tree-sitter.el  -*- lexical-binding: t; -*-

;; Copyright (C) 2018 Karl Otness

;; This file is part of tree-sitter.el.

;; tree-sitter.el is free software: you can redistribute it and/or
;; modify it under the terms of the GNU General Public License as
;; published by the Free Software Foundation, either version 3 of the
;; License, or (at your option) any later version.

;; tree-sitter.el is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
;; General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with tree-sitter.el. If not, see
;; <https://www.gnu.org/licenses/>.

;;; Commentary:

;; Live incremental parsing for buffers. Enable in a buffer using
;; `tree-sitter-live-setup'.

;;; Code:
(require 'tree-sitter-defs)
(require 'tree-sitter-module)

(defvar-local tree-sitter-live--parser nil
  "Tree-sitter parser used to parse this buffer.")
(defvar-local tree-sitter-live-tree nil
  "Tree-sitter tree for the current buffer.")
;; Store [start_byte old_end_byte start_point old_end_point]
(defvar-local tree-sitter-live--before-change nil
  "Internal value for tracking old buffer locations")

(defun tree-sitter-live--before-change (beg end)
  "Hook for `before-change-functions'."
  (save-excursion
    (let ((start-byte (position-bytes beg))
          (old-end-byte (position-bytes end))
          (start-point (tree-sitter-position-to-point beg))
          (old-end-point (tree-sitter-position-to-point end)))
      (aset tree-sitter-live--before-change 0 start-byte)
      (aset tree-sitter-live--before-change 1 old-end-byte)
      (aset tree-sitter-live--before-change 2 start-point)
      (aset tree-sitter-live--before-change 3 old-end-point))))

(defun tree-sitter-live--after-change (beg end pre-len)
  "Hook for `after-change-functions'."
  (let ((start-byte (aref tree-sitter-live--before-change 0))
        (old-end-byte (aref tree-sitter-live--before-change 1))
        (new-end-byte (position-bytes end))
        (start-point (aref tree-sitter-live--before-change 2))
        (old-end-point (aref tree-sitter-live--before-change 3))
        (new-end-point (tree-sitter-position-to-point end)))
    (tree-sitter-tree-edit tree-sitter-live-tree
                           start-byte old-end-byte new-end-byte
                           start-point old-end-point new-end-point)
    (setq tree-sitter-live-tree
          (tree-sitter-parser-parse-buffer tree-sitter-live--parser
                                           (current-buffer)
                                           tree-sitter-live-tree))))

(defun tree-sitter-live-setup (language)
  "Enable tree-sitter-live for LANGUAGE in current buffer.
LANGUAGE must be a tree-sitter-language record."
  (setq tree-sitter-live--parser (tree-sitter-parser-new))
  (tree-sitter-parser-set-language tree-sitter-live--parser language)
  (setq tree-sitter-live-tree
        (tree-sitter-parser-parse-buffer tree-sitter-live--parser
                                         (current-buffer)))
  (setq tree-sitter-live--before-change (make-vector 4 0))
  (add-hook 'before-change-functions #'tree-sitter-live--before-change nil t)
  (add-hook 'after-change-functions #'tree-sitter-live--after-change nil t))

(provide 'tree-sitter-live)
;;; tree-sitter-defs.el ends here