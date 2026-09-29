/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        'ps-blue': '#0095ff',
        'ps-blue-glow': 'rgba(0, 149, 255, 0.4)',
        'ps-black': '#08080a',
        'ps-surface': '#101014',
        'ps-card': 'rgba(24, 24, 28, 0.8)',
        'ps-border': 'rgba(255, 255, 255, 0.08)',
      },
      borderRadius: {
        'ps-xl': '1rem',
        'ps-2xl': '1.5rem',
        'ps-3xl': '2rem',
      },
      fontFamily: {
        'ps5': ["Inter", "system-ui", "sans-serif"],
      }
    },
  },
  plugins: [],
}
