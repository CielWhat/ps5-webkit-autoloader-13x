import React from 'react'
import { cn } from '../../utils/helpers'

const Modal = ({ show, title, children, onClose, footer }) => {
  if (!show) return null
  return (
    <div className="fixed inset-0 z-[1000] flex items-center justify-center p-4 md:p-8 animate-in fade-in duration-200">
      <div className="absolute inset-0 bg-black/80" onClick={onClose} />
      <div className="relative glass-panel w-full max-w-xl p-6 md:p-10 rounded-ps-3xl border-white/10 shadow-[0_0_100px_rgba(0,0,0,0.5)] animate-in zoom-in duration-300 overflow-hidden">
        <h3 className="text-2xl md:text-3xl font-black text-white uppercase italic tracking-tighter mb-6 md:mb-8">{title}</h3>
        <div className="text-zinc-300 mb-8 md:mb-10 text-lg md:text-xl font-medium leading-relaxed">
          {children}
        </div>
        {footer && <div className="flex gap-4">{footer}</div>}
      </div>
    </div>
  )
}

export default Modal
