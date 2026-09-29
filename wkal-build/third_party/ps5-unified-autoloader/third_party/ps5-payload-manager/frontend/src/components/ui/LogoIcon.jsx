import React from 'react'

export default function LogoIcon({ className = "w-6 h-6" }) {
  return (
    <svg 
      viewBox="0 0 24 24" 
      fill="none" 
      stroke="currentColor" 
      strokeWidth="0" 
      className={className}
      xmlns="http://www.w3.org/2000/svg"
    >
      {/* Sidebar Line */}
      <rect x="2" y="4" width="2.5" height="16" rx="1" fill="currentColor" />
      
      {/* Payload Grid */}
      <rect x="7.5" y="7" width="4" height="4" rx="1" fill="currentColor" />
      <rect x="13" y="7" width="4" height="4" rx="1" fill="currentColor" />
      <rect x="18.5" y="7" width="4" height="4" rx="1" fill="currentColor" />
      
      <rect x="7.5" y="13" width="4" height="4" rx="1" fill="currentColor" />
      <rect x="13" y="13" width="4" height="4" rx="1" fill="currentColor" />
      <rect x="18.5" y="13" width="4" height="4" rx="1" fill="currentColor" />
    </svg>
  )
}
