import React, { createContext, useState } from "react";

export const SidebarContext = createContext();

export function SidebarProvider({ children }) {
  const [sidebarWidth, setSidebarWidth] = useState(240);

  return (
    <SidebarContext.Provider value={{ sidebarWidth, setSidebarWidth }}>
      {children}
    </SidebarContext.Provider>
  );
}
